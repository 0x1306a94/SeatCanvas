//
//  GestureManager.ts
//  SeatCanvas
//
//  Created by king on 2026/5/26.
//

/**
 * GestureState enum matching C++ kk::gesture::GestureState.
 */
export const enum GestureState {
    POSSIBLE = 0,
    BEGAN = 1,
    CHANGED = 2,
    ENDED = 3,
    CANCELLED = 4,
}

enum DeviceType {
    TOUCH = 0,
    MOUSE = 1,
}

export const MIN_ZOOM = 0.001;
export const MAX_ZOOM = 1000.0;

export class GestureManager {
    private scaleY = 1.0;
    private pinchTimeout = 150;
    private timer: number | undefined;
    private scaleStartZoom = 1.0;

    private lastEventTime = 0;
    private lastDeltaY = 0;
    private timeThreshold = 50;
    private deltaYThreshold = 50;
    private deltaYChangeThreshold = 10;
    private mouseWheelRatio = 800;
    private touchWheelRatio = 100;

    // Touchpad inertia detection
    private touchpadConsecutiveDecreasing = 0;
    private lastTouchpadDeltaMag = 0;
    private touchpadInertiaEnded = false;
    private touchpadCooldownUntil = 0;

    private panDx = 0;
    private panDy = 0;
    private pinchCenterX = 0;
    private pinchCenterY = 0;

    public zoom = 1.0;
    public offsetX = 0;
    public offsetY = 0;

    // Mouse drag state
    private mouseDragging = false;
    private mousePanBegan = false;
    private mouseStartX = 0;
    private mouseStartY = 0;
    private mousePrevX = 0;
    private mousePrevY = 0;
    private mouseAccumDx = 0;
    private mouseAccumDy = 0;

    // Touch gesture state
    private touchActive = false;
    private touchPinching = false;
    private touchPanBegan = false;
    private touchStartX = 0;
    private touchStartY = 0;
    private touchPrevX = 0;
    private touchPrevY = 0;
    private touchAccumDx = 0;
    private touchAccumDy = 0;
    private initialPinchDistance = 0;
    private pinchStartCenterX = 0;
    private pinchStartCenterY = 0;
    private pinchCurrentCenterX = 0;
    private pinchCurrentCenterY = 0;

    private handlePanCallback: ((state: GestureState, tx: number, ty: number, ts: number) => void) | null = null;
    private handlePinchCallback: ((state: GestureState, scale: number, cx: number, cy: number) => void) | null = null;
    private handleTapCallback: ((x: number, y: number) => void) | null = null;

    constructor() {
        this.handlePanCallback = null;
        this.handlePinchCallback = null;
        this.handleTapCallback = null;
    }

    public setPanCallback(cb: (state: GestureState, tx: number, ty: number, ts: number) => void) {
        this.handlePanCallback = cb;
    }

    public setPinchCallback(cb: (state: GestureState, scale: number, cx: number, cy: number) => void) {
        this.handlePinchCallback = cb;
    }

    public setTapCallback(cb: (x: number, y: number) => void) {
        this.handleTapCallback = cb;
    }

    private getDeviceType(event: WheelEvent): DeviceType {
        const now = Date.now();
        const timeDifference = now - this.lastEventTime;
        const deltaYChange = Math.abs(event.deltaY - this.lastDeltaY);
        let isTouchpad = false;
        if (event.deltaMode === event.DOM_DELTA_PIXEL && timeDifference < this.timeThreshold) {
            if (Math.abs(event.deltaY) < this.deltaYThreshold && deltaYChange < this.deltaYChangeThreshold) {
                isTouchpad = true;
            }
        }
        this.lastEventTime = now;
        this.lastDeltaY = event.deltaY;
        return isTouchpad ? DeviceType.TOUCH : DeviceType.MOUSE;
    }

    public onWheel(event: WheelEvent) {
        const deviceType = this.getDeviceType(event);
        const wheelRatio = (deviceType === DeviceType.MOUSE ? this.mouseWheelRatio : this.touchWheelRatio);
        const timestampMs = event.timeStamp;

        if (!event.deltaY || (!event.ctrlKey && !event.metaKey)) {
            // Pan
            let deltaX = event.deltaX * window.devicePixelRatio;
            let deltaY = event.deltaY * window.devicePixelRatio;
            if (event.shiftKey && event.deltaX === 0 && event.deltaY !== 0) {
                deltaX = event.deltaY * window.devicePixelRatio;
                deltaY = 0;
            }

            // Touchpad inertia detection: browser generates synthetic events after
            // fingers are lifted. Detect the monotonic decay pattern and end the gesture
            // early so the C++ spring-back can start immediately.
            if (deviceType === DeviceType.TOUCH) {
                const now = Date.now();
                const deltaMag = Math.abs(deltaX) + Math.abs(deltaY);
                if (this.touchpadInertiaEnded && now < this.touchpadCooldownUntil) {
                    // Ignore residual inertia events during cooldown.
                    return;
                }
                if (this.touchpadInertiaEnded && now >= this.touchpadCooldownUntil) {
                    // Cooldown over, user may have started a new scroll.
                    this.touchpadInertiaEnded = false;
                    this.touchpadConsecutiveDecreasing = 0;
                    this.lastTouchpadDeltaMag = 0;
                }
                if (deltaMag > 0 && deltaMag < this.lastTouchpadDeltaMag) {
                    this.touchpadConsecutiveDecreasing++;
                } else {
                    this.touchpadConsecutiveDecreasing = 0;
                }
                this.lastTouchpadDeltaMag = deltaMag;
                // 4+ consecutive decreasing deltas indicates browser inertia rather than
                // active finger control. End the gesture immediately.
                if (this.touchpadConsecutiveDecreasing >= 4) {
                    if (this.timer !== undefined) {
                        this.handlePanCallback?.(GestureState.CHANGED, this.panDx + deltaX,
                                                this.panDy + deltaY, timestampMs);
                        clearTimeout(this.timer);
                        this.timer = undefined;
                        this.panDx = 0;
                        this.panDy = 0;
                        this.handlePanCallback?.(GestureState.ENDED, 0, 0, timestampMs);
                    }
                    this.touchpadInertiaEnded = true;
                    this.touchpadCooldownUntil = now + 300;
                    this.touchpadConsecutiveDecreasing = 0;
                    this.lastTouchpadDeltaMag = 0;
                    return;
                }
            }

            this.panDx += deltaX;
            this.panDy += deltaY;
            if (this.timer === undefined) {
                this.handlePanCallback?.(GestureState.BEGAN, this.panDx, this.panDy, timestampMs);
            } else {
                this.handlePanCallback?.(GestureState.CHANGED, this.panDx, this.panDy, timestampMs);
            }
            this.resetPanTimeout(event);
        } else {
            // Pinch zoom
            this.scaleY *= Math.exp(-(event.deltaY) / wheelRatio);
            const rect = (event.target as HTMLElement).getBoundingClientRect();
            this.pinchCenterX = (event.clientX - rect.left) * window.devicePixelRatio;
            this.pinchCenterY = (event.clientY - rect.top) * window.devicePixelRatio;
            if (this.timer === undefined) {
                this.handlePinchCallback?.(GestureState.BEGAN, this.scaleY, this.pinchCenterX, this.pinchCenterY);
            } else {
                this.handlePinchCallback?.(GestureState.CHANGED, this.scaleY, this.pinchCenterX, this.pinchCenterY);
            }
            this.resetPinchTimeout(event);
        }
    }

    public onMouseDown(event: MouseEvent, canvas: HTMLElement) {
        if (event.button !== 0) return;
        const rect = canvas.getBoundingClientRect();
        const pixelX = (event.clientX - rect.left) * window.devicePixelRatio;
        const pixelY = (event.clientY - rect.top) * window.devicePixelRatio;
        this.mouseDragging = true;
        this.mousePanBegan = false;
        this.mouseStartX = pixelX;
        this.mouseStartY = pixelY;
        this.mousePrevX = pixelX;
        this.mousePrevY = pixelY;
        this.mouseAccumDx = 0;
        this.mouseAccumDy = 0;
    }

    public onMouseMove(event: MouseEvent, canvas: HTMLElement) {
        if (!this.mouseDragging) return;
        const rect = canvas.getBoundingClientRect();
        const currX = (event.clientX - rect.left) * window.devicePixelRatio;
        const currY = (event.clientY - rect.top) * window.devicePixelRatio;
        const dx = currX - this.mousePrevX;
        const dy = currY - this.mousePrevY;
        this.mouseAccumDx -= dx;
        this.mouseAccumDy -= dy;
        this.mousePrevX = currX;
        this.mousePrevY = currY;
        const timestampMs = event.timeStamp;
        if (!this.mousePanBegan) {
            this.mousePanBegan = true;
            this.handlePanCallback?.(GestureState.BEGAN, 0, 0, timestampMs);
        }
        this.handlePanCallback?.(GestureState.CHANGED, this.mouseAccumDx, this.mouseAccumDy, timestampMs);
    }

    public onMouseUp(event: MouseEvent, canvas: HTMLElement) {
        if (!this.mouseDragging) return;
        this.mouseDragging = false;
        if (this.mousePanBegan) {
            const timestampMs = event.timeStamp;
            this.handlePanCallback?.(GestureState.ENDED, 0, 0, timestampMs);
        } else {
            this.handleTapCallback?.(this.mouseStartX, this.mouseStartY);
        }
        this.mouseAccumDx = 0;
        this.mouseAccumDy = 0;
    }

    public onTouchStart(event: TouchEvent, canvas: HTMLElement) {
        if (event.touches.length === 1) {
            // Single touch: possible pan, defer BEGAN until actual move
            this.touchActive = true;
            this.touchPinching = false;
            this.touchPanBegan = false;
            const touch = event.touches[0];
            const rect = canvas.getBoundingClientRect();
            this.touchStartX = (touch.clientX - rect.left) * window.devicePixelRatio;
            this.touchStartY = (touch.clientY - rect.top) * window.devicePixelRatio;
            this.touchPrevX = this.touchStartX;
            this.touchPrevY = this.touchStartY;
            this.touchAccumDx = 0;
            this.touchAccumDy = 0;
        } else if (event.touches.length === 2) {
            // Two fingers: pinch
            this.touchActive = true;
            this.touchPinching = true;
            const t0 = event.touches[0];
            const t1 = event.touches[1];
            const rect = canvas.getBoundingClientRect();
            const x0 = (t0.clientX - rect.left) * window.devicePixelRatio;
            const y0 = (t0.clientY - rect.top) * window.devicePixelRatio;
            const x1 = (t1.clientX - rect.left) * window.devicePixelRatio;
            const y1 = (t1.clientY - rect.top) * window.devicePixelRatio;
            const dx = x1 - x0;
            const dy = y1 - y0;
            this.initialPinchDistance = Math.sqrt(dx * dx + dy * dy);
            this.pinchStartCenterX = (x0 + x1) / 2;
            this.pinchStartCenterY = (y0 + y1) / 2;
            this.pinchCurrentCenterX = this.pinchStartCenterX;
            this.pinchCurrentCenterY = this.pinchStartCenterY;
            this.scaleY = 1.0;
            this.handlePinchCallback?.(GestureState.BEGAN, 1.0, this.pinchStartCenterX, this.pinchStartCenterY);
        }
    }

    public onTouchMove(event: TouchEvent, canvas: HTMLElement) {
        if (!this.touchActive) return;

        if (this.touchPinching && event.touches.length >= 2) {
            const t0 = event.touches[0];
            const t1 = event.touches[1];
            const rect = canvas.getBoundingClientRect();
            const x0 = (t0.clientX - rect.left) * window.devicePixelRatio;
            const y0 = (t0.clientY - rect.top) * window.devicePixelRatio;
            const x1 = (t1.clientX - rect.left) * window.devicePixelRatio;
            const y1 = (t1.clientY - rect.top) * window.devicePixelRatio;
            const dx = x1 - x0;
            const dy = y1 - y0;
            const distance = Math.sqrt(dx * dx + dy * dy);
            if (this.initialPinchDistance > 0) {
                this.scaleY = distance / this.initialPinchDistance;
            }
            this.pinchCurrentCenterX = (x0 + x1) / 2;
            this.pinchCurrentCenterY = (y0 + y1) / 2;
            this.handlePinchCallback?.(GestureState.CHANGED, this.scaleY, this.pinchCurrentCenterX, this.pinchCurrentCenterY);
        } else if (!this.touchPinching && event.touches.length === 1) {
            const touch = event.touches[0];
            const rect = canvas.getBoundingClientRect();
            const currX = (touch.clientX - rect.left) * window.devicePixelRatio;
            const currY = (touch.clientY - rect.top) * window.devicePixelRatio;
            const dx = currX - this.touchPrevX;
            const dy = currY - this.touchPrevY;
            this.touchAccumDx -= dx;
            this.touchAccumDy -= dy;
            this.touchPrevX = currX;
            this.touchPrevY = currY;
            const timestampMs = event.timeStamp;
            if (!this.touchPanBegan) {
                this.touchPanBegan = true;
                this.handlePanCallback?.(GestureState.BEGAN, 0, 0, timestampMs);
            }
            this.handlePanCallback?.(GestureState.CHANGED, this.touchAccumDx, this.touchAccumDy, timestampMs);
        }
    }

    public onTouchEnd(event: TouchEvent, canvas: HTMLElement) {
        if (event.touches.length === 0) {
            if (this.touchPinching) {
                this.handlePinchCallback?.(GestureState.ENDED, this.scaleY, this.pinchCurrentCenterX, this.pinchCurrentCenterY);
                this.scaleY = 1.0;
            } else if (this.touchActive) {
                if (this.touchPanBegan) {
                    const timestampMs = event.timeStamp;
                    this.handlePanCallback?.(GestureState.ENDED, 0, 0, timestampMs);
                } else {
                    this.handleTapCallback?.(this.touchStartX, this.touchStartY);
                }
                this.touchAccumDx = 0;
                this.touchAccumDy = 0;
            }
            this.touchActive = false;
            this.touchPinching = false;
        }
    }

    private resetPanTimeout(event: WheelEvent) {
        clearTimeout(this.timer);
        this.timer = window.setTimeout(() => {
            this.timer = undefined;
            this.panDx = 0;
            this.panDy = 0;
            this.handlePanCallback?.(GestureState.ENDED, 0, 0, event.timeStamp);
        }, this.pinchTimeout);
    }

    private resetPinchTimeout(event: WheelEvent) {
        clearTimeout(this.timer);
        this.timer = window.setTimeout(() => {
            this.timer = undefined;
            this.handlePinchCallback?.(GestureState.ENDED, this.scaleY, this.pinchCenterX, this.pinchCenterY);
            this.scaleY = 1.0;
        }, this.pinchTimeout);
    }

    public clearState() {
        this.scaleY = 1.0;
        this.timer = undefined;
        this.mouseDragging = false;
        this.mousePanBegan = false;
        this.touchActive = false;
        this.touchPinching = false;
        this.touchPanBegan = false;
        this.touchpadInertiaEnded = false;
        this.touchpadConsecutiveDecreasing = 0;
        this.touchpadCooldownUntil = 0;
    }
}
