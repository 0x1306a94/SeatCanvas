//
//  SeatCanvas.ts
//  SeatCanvas
//
//  Created by king on 2026/5/26.
//

import { GestureManager } from './GestureManager';
import { bindCanvasEvents, bindDevicePixelRatioChange, updateCanvasSize } from './common';
import * as types from './types';
import { SeatCanvasFont } from './SeatCanvasFont';
import createSeatCanvas from './wasm/libseatcanvas';
import { SeatCanvasModuleBinding } from './binding';

export interface ModuleOption {
    /**
     * Link to wasm file.
     */
    locateFile?: (file: 'libseatcanvas.wasm') => string;
}

/**
* Initialize pag webassembly module.
*/
const SeatCanvasInit = (moduleOption: ModuleOption = {}): Promise<types.SeatCanvasModule> =>
    createSeatCanvas(moduleOption)
        .then((module: any) => {
            SeatCanvasModuleBinding(module);
            SeatCanvasFont.registerFallbackFontNames();
            return module;
        })
        .catch((error: any) => {
            console.error(error);
            throw new Error('SeatCanvasInit fail! Please check .wasm file path valid.');
        });

export { SeatCanvasInit, types, SeatCanvasFont };


/**
 * SeatCanvas Web 端高层封装
 *
 * 负责 canvas 尺寸同步、手势绑定、DPR 监听和渲染器生命周期管理
 * 典型用法：{@link SeatCanvasInit} → `init()` → `start()` → `destroy()`
 */
export class SeatCanvasApp {
    private module: types.SeatCanvasModule | null = null;
    private renderer: types.SeatCanvasRenderer | null = null;
    private delegate: types.SeatCanvasDelegate | null = null;
    private gestureManager: GestureManager = new GestureManager();
    private canvas: HTMLCanvasElement | null = null;
    private running: boolean = false;
    private eventCleanup: (() => void) | null = null;
    private resizeCleanup: (() => void) | null = null;

    /**
     * @param canvasID canvas 元素的 CSS 选择器，如 `'#seat-canvas'`
     */
    constructor(private canvasID: string) {
    }

    /**
     * 绑定已加载的 WASM 模块并创建渲染器
     * 会自动设置 canvas 尺寸、手势监听和 resize/DPR 监听
     *
     * @param module 由 {@link SeatCanvasInit} 返回的模块实例
     * @throws 找不到对应 canvas 元素时抛出异常
     */
    public init(module: types.SeatCanvasModule) {
        this.module = module;
        const canvas = document.querySelector(this.canvasID) as HTMLCanvasElement;
        if (!canvas) {
            throw new Error(`Canvas element "${this.canvasID}" not found`);
        }
        this.canvas = canvas;

        updateCanvasSize(canvas);

        this.renderer = this.module.SeatCanvasRenderer.MakeFrom(this.canvasID);
        this.delegate = this.renderer.getDelegate();

        this.setupGestureHandlers();
        this.setupResizeHandler();
    }

    /**
     * 获取底层渲染器实例，用于调用座位数据、样式等 API
     * 未调用 {@link init} 时返回 `null`
     */
    public getRenderer(): types.SeatCanvasRenderer | null {
        return this.renderer;
    }

    /**
     * 获取事件委托，用于注册底图加载、点击、视口变化等回调
     * 未调用 {@link init} 时返回 `null`
     */
    public getDelegate(): types.SeatCanvasDelegate | null {
        return this.delegate;
    }

    /**
     * 启动渲染循环
     * 帧驱动由 C++ DisplayLink 内部完成，无需 JS 侧 requestAnimationFrame
     */
    public start() {
        if (!this.renderer || this.running) {
            return;
        }
        this.running = true;
        this.renderer.start();
    }

    /** 停止渲染循环 */
    public stop() {
        if (!this.renderer) {
            return;
        }
        this.running = false;
        this.renderer.stop();
    }

    /**
     * 释放所有资源：停止渲染、解绑事件监听、清空引用
     * 调用后不可再使用此实例，需重新 {@link init}
     */
    public destroy() {
        this.stop();
        this.eventCleanup?.();
        this.resizeCleanup?.();
        this.eventCleanup = null;
        this.resizeCleanup = null;
        this.renderer = null;
        this.delegate = null;
        this.canvas = null;
        this.module = null;
    }

    /**
     * 从 SVG 字符串加载底图
     *
     * @param svgData SVG 文件内容
     * @param parseConfigJSON 可选解析配置 JSON，见 {@link types.SVGBaseMapParseConfig}
     * @returns 解析成功返回 `true`
     */
    public loadBaseMapFromSVG(svgData: string, parseConfigJSON?: string): boolean {
        if (!this.renderer) {
            return false;
        }
        return this.renderer.loadBaseMapFromSVG(svgData, parseConfigJSON ?? '');
    }

    /**
     * 注册调试用的 delegate 回调（输出到 console）
     * 生产环境请自行通过 {@link getDelegate} 注册业务回调
     */
    public setupDebugCallbacks() {
        if (!this.delegate) {
            return;
        }
        this.delegate.setDidLoadBaseMapCallback(() => {
            console.log('[SeatCanvas] Base map loaded');
        });
        this.delegate.setDidTapZoneCallback((zoneId: string) => {
            console.log('[SeatCanvas] Tapped zone:', zoneId);
        });
        this.delegate.setDidTapSeatCallback((zoneId: string, seatId: string) => {
            console.log('[SeatCanvas] Tapped seat:', seatId, 'in zone:', zoneId);
            return true;
        });
    }

    private setupGestureHandlers() {
        if (!this.canvas || !this.renderer) {
            return;
        }

        this.gestureManager.setPanCallback((state, tx, ty, ts) => {
            this.renderer?.handlePan(state, tx, ty, ts);
        });

        this.gestureManager.setPinchCallback((state, scale, cx, cy) => {
            this.renderer?.handlePinch(state, scale, cx, cy);
        });

        this.gestureManager.setTapCallback((x, y) => {
            this.renderer?.handleTap(x, y);
        });

        this.eventCleanup = bindCanvasEvents(this.canvas, this.gestureManager);
    }

    private setupResizeHandler() {
        if (!this.canvas) {
            return;
        }

        const onResize = () => {
            updateCanvasSize(this.canvas!);
            this.renderer?.updateSize();
            this.renderer?.invalidateContent();
        };

        const container = this.canvas.parentElement;
        const cleanups: Array<() => void> = [];

        if (container) {
            const resizeObserver = new ResizeObserver(onResize);
            resizeObserver.observe(container);
            cleanups.push(() => resizeObserver.disconnect());
        }

        window.addEventListener('resize', onResize);
        cleanups.push(() => window.removeEventListener('resize', onResize));

        cleanups.push(bindDevicePixelRatioChange(onResize));

        this.resizeCleanup = () => {
            for (const cleanup of cleanups) {
                cleanup();
            }
        };
    }
}
