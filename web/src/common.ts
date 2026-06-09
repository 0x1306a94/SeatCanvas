//
//  common.ts
//  SeatCanvas
//
//  Created by king on 2026/5/26.
//

import { GestureManager } from './GestureManager';

export type CanvasEventCleanup = () => void;

/**
 * Bind DOM events on a canvas element to the gesture manager.
 * Returns a cleanup function to remove all listeners.
 */
export function bindCanvasEvents(canvas: HTMLElement, gestureManager: GestureManager): CanvasEventCleanup {
    if (!canvas) {
        return () => {};
    }

    const wheelHandler = (e: WheelEvent) => {
        e.preventDefault();
        gestureManager.onWheel(e);
    };

    const mouseDownHandler = (e: MouseEvent) => {
        gestureManager.onMouseDown(e, canvas);
    };

    const mouseMoveHandler = (e: MouseEvent) => {
        gestureManager.onMouseMove(e, canvas);
    };

    const mouseUpHandler = (e: MouseEvent) => {
        gestureManager.onMouseUp(e, canvas);
    };

    const touchStartHandler = (e: TouchEvent) => {
        e.preventDefault();
        gestureManager.onTouchStart(e, canvas);
    };

    const touchMoveHandler = (e: TouchEvent) => {
        e.preventDefault();
        gestureManager.onTouchMove(e, canvas);
    };

    const touchEndHandler = (e: TouchEvent) => {
        gestureManager.onTouchEnd(e, canvas);
    };

    canvas.addEventListener('wheel', wheelHandler, { passive: false });
    canvas.addEventListener('mousedown', mouseDownHandler);
    window.addEventListener('mousemove', mouseMoveHandler);
    window.addEventListener('mouseup', mouseUpHandler);
    canvas.addEventListener('touchstart', touchStartHandler, { passive: false });
    canvas.addEventListener('touchmove', touchMoveHandler, { passive: false });
    canvas.addEventListener('touchend', touchEndHandler);
    canvas.addEventListener('touchcancel', touchEndHandler);

    return () => {
        canvas.removeEventListener('wheel', wheelHandler);
        canvas.removeEventListener('mousedown', mouseDownHandler);
        window.removeEventListener('mousemove', mouseMoveHandler);
        window.removeEventListener('mouseup', mouseUpHandler);
        canvas.removeEventListener('touchstart', touchStartHandler);
        canvas.removeEventListener('touchmove', touchMoveHandler);
        canvas.removeEventListener('touchend', touchEndHandler);
        canvas.removeEventListener('touchcancel', touchEndHandler);
    };
}

/**
 * Setup canvas resize handling based on container dimensions and device pixel ratio.
 */
export function updateCanvasSize(canvas: HTMLCanvasElement) {
    const container = canvas.parentElement;
    if (!container) {
        return;
    }
    const screenRect = container.getBoundingClientRect();
    const scaleFactor = window.devicePixelRatio;
    canvas.width = screenRect.width * scaleFactor;
    canvas.height = screenRect.height * scaleFactor;
    canvas.style.width = screenRect.width + "px";
    canvas.style.height = screenRect.height + "px";
}

/**
 * Listen for device pixel ratio changes (e.g. moving window across displays).
 * Returns a cleanup function.
 */
export function bindDevicePixelRatioChange(onChange: () => void): () => void {
    let dppx = window.devicePixelRatio;
    let mediaQuery: MediaQueryList | null = null;

    const onDprChange = () => {
        dppx = window.devicePixelRatio;
        onChange();
        setupListener();
    };

    const setupListener = () => {
        if (mediaQuery) {
            mediaQuery.removeEventListener('change', onDprChange);
        }
        mediaQuery = window.matchMedia(`(resolution: ${dppx}dppx)`);
        mediaQuery.addEventListener('change', onDprChange);
    };

    setupListener();

    return () => {
        mediaQuery?.removeEventListener('change', onDprChange);
        mediaQuery = null;
    };
}
