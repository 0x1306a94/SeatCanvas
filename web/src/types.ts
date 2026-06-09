//
//  types.ts
//  SeatCanvas
//
//  Created by king on 2026/5/26.
//

/* global EmscriptenModule */

/** RGBA 颜色，各分量取值范围 0~1 */
export interface ColorRGBA {
    /** 红色分量 */
    r: number;
    /** 绿色分量 */
    g: number;
    /** 蓝色分量 */
    b: number;
    /** 透明度，0 为全透明，1 为不透明 */
    a: number;
}

/**
 * SeatCanvas WebAssembly 模块实例
 * 由 `SeatCanvasInit` 加载后获得
 */
export interface SeatCanvasModule extends EmscriptenModule {
    SeatCanvasRenderer: {
        /** 根据 canvas 选择器创建渲染器 */
        MakeFrom(canvasID: string): SeatCanvasRenderer;
        /**
         * 通过 FreeType 注册 fallback 字体字节数据。
         * 需在 SeatCanvasInit() 之后、MakeFrom() 创建渲染器之前调用。
         * @returns 正文字体注册成功返回 true
         */
        RegisterFonts(textFontData: Uint8Array, emojiFontData?: Uint8Array): boolean;
    };
    GL: any;
    HEAPU8: Uint8Array;
    module: SeatCanvasModule;
}

/**
 * 座位图渲染器，WASM 侧的核心渲染 API
 */
export interface SeatCanvasRenderer {
    /** 启动渲染循环（内部由 C++ DisplayLink 驱动） */
    start(): void;
    /** 停止渲染循环 */
    stop(): void;
    /** 手动触发一帧绘制，通常无需调用 */
    draw(force?: boolean): void;

    /**
     * 从 SVG 字符串加载底图
     * @param svgData SVG 文件内容
     * @param parseConfigJSON 可选的底图解析配置 JSON 字符串，见 {@link SVGBaseMapParseConfig}
     * @returns 解析成功返回 `true`，失败返回 `false`
     */
    loadBaseMapFromSVG(svgData: string, parseConfigJSON?: string): boolean;

    /** 处理点击手势，坐标为 canvas 逻辑像素 */
    handleTap(x: number, y: number): void;
    /**
     * 处理平移手势
     * @param state 手势状态：0=Possible, 1=Began, 2=Changed, 3=Ended, 4=Cancelled
     * @param translationX 累计 X 偏移（逻辑像素）
     * @param translationY 累计 Y 偏移（逻辑像素）
     * @param timestampMs 事件时间戳（毫秒）
     */
    handlePan(state: number, translationX: number, translationY: number, timestampMs: number): void;
    /**
     * 处理缩放手势
     * @param state 手势状态，同 {@link SeatCanvasRenderer.handlePan}
     * @param scale 累计缩放比例
     * @param centerX 缩放中心 X（逻辑像素）
     * @param centerY 缩放中心 Y（逻辑像素）
     */
    handlePinch(state: number, scale: number, centerX: number, centerY: number): void;

    /** 设置背景色，各分量取值范围 0~1 */
    setBackgroundColor(r: number, g: number, b: number, a: number): void;
    /** 获取当前背景色 */
    getBackgroundColor(): ColorRGBA;

    /** 设置座位渲染尺寸（逻辑单位） */
    setSeatSize(size: number): void;
    /** 获取座位渲染尺寸 */
    getSeatSize(): number;

    /** 开启/关闭 Debug HUD（密度、缩放、FPS 等） */
    setDebugHUDEnabled(enabled: boolean): void;
    /** 查询 Debug HUD 是否开启 */
    isDebugHUDEnabled(): boolean;

    /** 当前缩放比例 */
    getZoomScale(): number;
    /** 最小允许缩放比例 */
    getMinimumZoomScale(): number;
    /** 最大允许缩放比例 */
    getMaximumZoomScale(): number;
    /** 当前内容偏移（逻辑像素） */
    getContentOffset(): { x: number; y: number };
    /** 当前视口在底图原始坐标系中的可见矩形 */
    getVisibleOriginalRect(): { x: number; y: number; width: number; height: number };
    /** 当前帧率 */
    getFPS(): number;

    /** 设置座位层开始渲染的缩放阈值 */
    setSeatRenderZoomThreshold(threshold: number): void;
    /** 获取座位层渲染缩放阈值 */
    getSeatRenderZoomThreshold(): number;

    /**
     * 缩放并平移视口，使指定矩形区域可见
     * @param left 目标矩形左边界（底图原始坐标）
     * @param top 目标矩形上边界
     * @param right 目标矩形右边界
     * @param bottom 目标矩形下边界
     * @param animated 是否使用动画过渡
     * @param padding 内边距（逻辑像素）
     * @param durationMs 动画时长（毫秒），仅 `animated=true` 时有效
     */
    zoomToRect(left: number, top: number, right: number, bottom: number,
               animated: boolean, padding: number, durationMs: number): void;

    /** 注册 pricecode 列表，返回的索引用于 {@link SeatData.pricecode} */
    registerPricecodes(pricecodes: string[]): void;
    /** 为指定区域设置座位数据 */
    setSeatData(zoneId: string, seats: SeatData[]): void;
    /** 清除所有座位数据 */
    clearSeatData(): void;
    /** 批量更新座位状态 */
    updateSeatStatuses(updates: SeatStatusUpdate[]): void;
    /** 按区域顺序批量更新座位状态（数组下标对应座位顺序） */
    updateSeatStatusesForZone(zoneId: string, statuses: Uint32Array): void;
    /** 设置当前选中的座位 ID 列表（全量替换） */
    setSelectedSeatIds(seatIds: string[]): void;
    /** 增量更新选中座位 ID */
    updateSelectedSeatIds(added: string[], removed: string[]): void;
    /** 从 JSON 字符串应用座位样式配置 */
    applySeatStyleJSONConfig(jsonString: string): void;
    /**
     * 更新各区域的备用颜色（主视图）
     * @param colors 键为 zoneId，值为 `#AARRGGBB` 或 `#RRGGBB` 格式
     */
    updateSeatZoneAlternateColors(colors: Record<string, string>): void;
    /** 更新各区域的备用颜色（小地图） */
    updateMiniMapZoneAlternateColors(colors: Record<string, string>): void;
    /**
     * 同步 canvas 尺寸与设备像素比到渲染器
     * @returns 尺寸发生变化时返回 `true`
     */
    updateSize(): boolean;
    /** 标记内容需要重绘 */
    invalidateContent(): void;

    /** 获取事件委托，用于注册各类回调 */
    getDelegate(): SeatCanvasDelegate;
}

/**
 * 渲染器事件委托，回调由 C++ 侧触发
 */
export interface SeatCanvasDelegate {
    /** 底图加载完成 */
    setDidLoadBaseMapCallback(cb: () => void): void;
    /** 底图卸载 */
    setDidUnloadBaseMapCallback(cb: () => void): void;
    /** 缩放级别配置更新 */
    setDidUpdateZoomLevelConfigCallback(cb: (event: ZoomLevelConfigEvent) => void): void;
    /** 点击区域 */
    setDidTapZoneCallback(cb: (zoneId: string) => void): void;
    /**
     * 点击座位
     * @returns 返回 `true` 表示需要重绘
     */
    setDidTapSeatCallback(cb: (zoneId: string, seatId: string) => boolean): void;
    /** 视口即将开始拖拽 */
    setViewportWillBeginDraggingCallback(cb: (event: ViewportEvent) => void): void;
    /** 视口滚动中 */
    setViewportDidScrollCallback(cb: (event: ViewportEvent) => void): void;
    /** 视口结束拖拽 */
    setViewportDidEndDraggingCallback(cb: (event: ViewportEvent, willDecelerate: boolean) => void): void;
    /** 视口结束惯性减速 */
    setViewportDidEndDeceleratingCallback(cb: (event: ViewportEvent) => void): void;
    /** 视口即将开始缩放 */
    setViewportWillBeginZoomingCallback(cb: (event: ViewportEvent) => void): void;
    /** 视口缩放中 */
    setViewportDidZoomCallback(cb: (event: ViewportEvent) => void): void;
    /** 视口结束缩放 */
    setViewportDidEndZoomingCallback(cb: (event: ViewportEvent) => void): void;
    /** 视口滚动动画结束 */
    setViewportDidEndScrollingAnimationCallback(cb: (event: ViewportEvent) => void): void;
}

/** 视口状态变化事件 */
export interface ViewportEvent {
    /** 当前缩放比例 */
    zoomScale: number;
    /** 内容 X 偏移 */
    contentOffsetX: number;
    /** 内容 Y 偏移 */
    contentOffsetY: number;
    /** 视口在底图原始坐标系中的可见矩形 */
    visibleOriginalRect: { x: number; y: number; width: number; height: number };
}

/** 缩放级别配置变化事件 */
export interface ZoomLevelConfigEvent {
    /** 各级别对应的缩放阈值 */
    zoomLevels: { seat: number; row: number; zone: number; venue: number };
    /** 最小缩放比例 */
    minimumZoomScale: number;
    /** 最大缩放比例 */
    maximumZoomScale: number;
    /** 当前缩放比例 */
    zoomScale: number;
}

/** 单个座位的数据 */
export interface SeatData {
    /** 座位唯一标识 */
    seatId: string;
    /** X 坐标（底图原始坐标系） */
    x: number;
    /** Y 坐标（底图原始坐标系） */
    y: number;
    /** 旋转角度（度），默认 0 */
    rotation?: number;
    /** pricecode 字符串，需先通过 {@link SeatCanvasRenderer.registerPricecodes} 注册 */
    pricecode?: string;
}

/** 座位状态更新项 */
export interface SeatStatusUpdate {
    /** 座位 ID */
    seatId: string;
    /** 状态值（由业务层定义语义） */
    status: number;
}

/** 底图解析配置基类 */
export interface BaseMapParseConfig {
}

/** SVG 底图解析配置，序列化为 JSON 后传入 {@link SeatCanvasRenderer.loadBaseMapFromSVG} */
export interface SVGBaseMapParseConfig extends BaseMapParseConfig {
    /** SVG 元素上用于识别 zoneId 的属性名列表，按优先级排列 */
    zoneIdAttributeNames: string[];
}
