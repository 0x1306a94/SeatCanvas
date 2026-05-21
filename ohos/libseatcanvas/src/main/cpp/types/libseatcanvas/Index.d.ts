import resourceManager from '@ohos.resourceManager';

export declare namespace seatcanvas {
  export class JFont {
    static RegisterFontFromPath(fontPath: string, ttcIndex?: number, fontFamily?: string, fontStyle?: string): JFont;

    static RegisterFontFromAsset(manager: resourceManager.ResourceManager, fileName: string, ttcIndex?: number,
      fontFamily?: string, fontStyle?: string): JFont;

    static UnregisterFont(font: JFont);

    static SetFallbackFontPaths(fontPath: Array<string>): void;

    constructor(fontFamily?: string, fontStyle?: string);

    fontFamily: string;

    fontStyle: string;
  }

  export enum GestureState {
    POSSIBLE = 0,
    BEGAN = 1,
    CHANGED = 2,
    ENDED = 3,
    CANCELLED = 4,
  }

  export interface Rect {
    x: number;
    y: number;
    width: number;
    height: number;
  }

  export interface SeatZoneColor {
    zoneId: string;
    alternateColor?: string;
  }

  export interface SeatData {
    seatId: string;
    x: number;
    y: number;
    rotation?: number;
    pricecode?: string;
  }

  export interface SeatStatusUpdate {
    seatId: string;
    status: number;
  }

  export class JSeatRenderStyleId {
    /**
     * 生成与 C++ 渲染器相同格式的座位样式 ID。
     * @param pricecode 价档 code；空字符串表示无价档槽位。
     * @param status 业务自定义座位状态。
     * @param selected 是否选中。
     */
    static compose(pricecode: string, status: number, selected: boolean): string;
  }

  export interface ZoomLevel {
    seat: number;
    row: number;
    zone: number;
    venue: number;
  }

  export interface SeatCanvasViewport {
    zoomScale: number;
    contentOffsetX: number;
    contentOffsetY: number;
    visibleOriginalRect: Rect;
  }

  export interface SeatCanvasBaseMapLoadedEvent {
    baseMapWidth: number;
    baseMapHeight: number;
    zoomLevels: ZoomLevel;
    minimumZoomScale: number;
    maximumZoomScale: number;
    zoomScale: number;
    visibleOriginalRect: Rect;
  }

  export class JRendererCore {
    static InitSystemProperties(density: number, fontScale: number): void;

    /**
     * 从 Assets 中异步的解析底图
     * @param manager 资源管理器
     * @param name 资源名称
     * @param format 格式
     * @returns 解析结果 C++ 对象地址
     */
    static ParseBaseMapFromAssets(manager: resourceManager.ResourceManager, name: string,
      format: string, parseConfigJSON?: string | null): Promise<number>;

    /**
     * 当前实例唯一标识符，用于绑定 XComponent
     * @returns 唯一标识符
     */
    uniqueID(): string;

    /**
     * 加载底图
     * @param nativePtr 由 ParseBaseMapFromAssets 返回的 C++ 对象地址，调用次函数后 C++ 对象将被释放。
     */
    loadBaseMap(nativePtr: number);

    /**
     *  应用座位样式
     * @param config 座位样式配置
     */
    applySeatStyleJSONConfig(config: string | null);

    /**
     * 启动渲染loop
     */
    start(): void;

    /**
     * 停止渲染loop
     */
    stop(): void;

    /**
     * 设置画布背景色
     * @param value ColorInt（AARRGGBB），如 0xFFFFFFFF、0xff000000 等
     */
    setCanvasColor(value: number): void;

    /**
     * 获取画布背景色
     * @returns ColorInt（AARRGGBB）
     */
    getCanvasColor(): number;

    /**
     * 获取座位大小
     * @returns 座位大小
     */
    getSeatSize(): number;

    /**
     * 设置座位大小
     * @param seatSize 座位大小
     */
    setSeatSize(seatSize: number): void;

    /**
     * 获取当前内容允许的最小缩放比例
     */
    getMinimumZoomScale(): number;

    /**
     * 获取当前内容允许的最大缩放比例
     */
    getMaximumZoomScale(): number;

    /**
     * 获取当前缩放级别
     */
    getZoomScale(): number;

    /**
     * 获取当前显示范围（原始坐标系）
     */
    getVisibleOriginalRect(): Rect;

    /**
     * 查找与指定矩形相交的区域 ID 列表（原始坐标系）
     * @param rect 查询矩形，通常配合 getVisibleOriginalRect 使用
     */
    getZoneIdsInOriginalRect(rect: Rect): string[];

    zoomLevel(): ZoomLevel;

    getSeatRenderZoomThreshold(): number;

    setSeatRenderZoomThreshold(threshold: number): void;

    /**
     * 释放内部 C++ 资源， 调用后将不能在继续使用当前对象
     */
    release(): void;

    /**
     * 处理点击手势
     * @param x X 方向移动量（px单位）
     * @param y Y 方向移动量（px单位）
     */
    handleTap(x: number, y: number);

    /**
     * 处理滑动手势
     * @param state 手势状态
     * @param tx X 方向移动量（px单位）
     * @param ty Y 方向移动量（px单位）
     */
    handlePan(state: GestureState, tx: number, ty: number);

    /**
     * 处理缩放手势
     * @param state 手势状态
     * @param scale 缩放比
     * @param cx 缩放 X 中心（px单位）
     * @param cy 缩放 Y 中心（px单位）
     */
    handlePinch(state: GestureState, scale: number, cx: number, cy: number);

    /**
     * 缩放到指定区域
     * @param bounds 原始内容的区域
     * @param animated 是否需要动画
     * @param padding 区域周围的边距（在内容坐标系中），默认为 0
     * @param duration 动画持续时间（毫秒），仅在 animated 为 true 时有效，默认 300ms
     */
    zoomToRect(bounds: Rect, animated: boolean, padding: number, duration: number);

    /**
     * 座位点击；返回 true 表示需要重绘。
     */
    setDidTapSeatCallback(callback: ((zoneId: string, seatId: string) => boolean) | null);

    setDidTapZoneCallback(callback: ((zoneId: string) => void) | null);

    setViewportWillBeginDraggingCallback(callback: ((viewport: SeatCanvasViewport) => void) | null);

    setViewportDidScrollCallback(callback: ((viewport: SeatCanvasViewport) => void) | null);

    setViewportDidEndDraggingCallback(callback: ((viewport: SeatCanvasViewport, decelerate: boolean) => void) | null);

    setViewportDidEndDeceleratingCallback(callback: ((viewport: SeatCanvasViewport) => void) | null);

    setViewportWillBeginZoomingCallback(callback: ((viewport: SeatCanvasViewport) => void) | null);

    setViewportDidZoomCallback(callback: ((viewport: SeatCanvasViewport) => void) | null);

    setViewportDidEndZoomingCallback(callback: ((viewport: SeatCanvasViewport) => void) | null);

    setViewportDidEndScrollingAnimationCallback(callback: ((viewport: SeatCanvasViewport) => void) | null);

    setDidLoadBaseMapCallback(callback: ((event: SeatCanvasBaseMapLoadedEvent) => void) | null);

    setDidUnloadBaseMapCallback(callback: (() => void) | null);

    updateSeatZoneAlternateColors(colors: SeatZoneColor[]);

    updateMiniMapZoneAlternateColors(colors: SeatZoneColor[]);

    updateSeats(zoneId: string, seats: SeatData[]);

    /** 注册价档表（load 前调用一次） */
    registerPricecodes(pricecodes: string[]);

    /** 批量更新单个座位 status */
    updateSeatStatuses(updates: SeatStatusUpdate[]);

    /** 批量更新某个 zone 内全部座位 status（数组下标与 updateSeats 顺序一致） */
    updateSeatStatusesForZone(zoneId: string, statuses: number[]);

    /** 全量替换选中座位 */
    setSelectedSeatIds(seatIds: string[]);

    /** 增量更新选中座位 */
    updateSelectedSeatIds(added: string[], removed: string[]);

    /**
     * 清除全部座位数据
     */
    clearSeatData(): void;
  }

}
