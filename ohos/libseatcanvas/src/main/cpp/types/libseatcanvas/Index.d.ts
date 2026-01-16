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

  export interface HitTestSeatRegionResult {
    /**
     * 区域ID
     */
    regionId: string;

    /**
     * 区域包围盒
     */
    bounds: Rect;
  }

  export class JRendererCore {
    static UpdateDensity(density: number): void;

    /**
     * 从 Assets 中异步的解析底图
     * @param manager 资源管理器
     * @param name 资源名称
     * @returns 解析结果 C++ 对象地址
     */
    static ParseBaseMapFromAssets(manager: resourceManager.ResourceManager, name: string): Promise<number>;

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
     * 启用Tiled 渲染模式
     * @param enable
     */
    enableTiled(enable: boolean);

    /**
     *  启用模糊优化，在缩放时
     * @param enable
     */
    enableZoomBlur(enable: boolean);

    /**
     * 启动渲染loop
     */
    start(): void;

    /**
     * 停止渲染loop
     */
    stop(): void;

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
     * 根据坐标查找座位区域
     * @param x X 坐标值（px单位）
     * @param y Y 坐标值（px单位）
     * @returns 区域信息
     */
    seatRegionByPoint(x: number, y: number): HitTestSeatRegionResult | undefined;

    /**
     * 缩放到指定区域
     * @param bounds 原始内容的区域，应该是由 seatRegionByPoint 获得的 bounds
     * @param animated 是否需要动画
     * @param padding 区域周围的边距（在内容坐标系中），默认为 0
     * @param duration 动画持续时间（毫秒），仅在 animated 为 true 时有效，默认 300ms
     */
    zoomToRect(bounds: Rect, animated: boolean, padding: number, duration: number);
  }

}