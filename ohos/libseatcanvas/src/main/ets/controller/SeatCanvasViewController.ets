import { seatcanvas } from 'libseatcanvas.so';
import { resourceManager } from '@kit.LocalizationKit';
import { display } from '@kit.ArkUI';
import { SeatCanvasInit } from '../utils/SeatCanvasInit';
import { BaseMapFormat } from '../utils/BaseMapFormat';
import { SeatCanvasRendererDelegate, SeatCanvasViewport } from './SeatCanvasRendererDelegate';
import { SeatCanvasZoomLevelConfigEvent } from './SeatCanvasZoomLevelConfigEvent';
import { BaseMapParseConfig } from './BaseMapParseConfig';
import { SeatData, SeatStatusUpdate } from './SeatData';
import { SeatZoneColor } from './SeatZoneColor';
import { ZoomLevel } from './ZoomLevel';

export class SeatCanvasViewController {
  private jView!: seatcanvas.JRendererCore;
  private view: WeakRef<ESObject> | null = null;
  private delegate: WeakRef<SeatCanvasRendererDelegate> | null = null;

  constructor() {
    SeatCanvasInit.Init();
    let displayInfo = display.getDefaultDisplaySync();
    let density = displayInfo.densityPixels
    let scaledDensity = displayInfo.scaledDensity
    seatcanvas.JRendererCore.InitSystemProperties(density, scaledDensity);

    this.jView = new seatcanvas.JRendererCore();
  }

  attachToView(view: WeakRef<ESObject>) {
    if (this.view != null && this.view != view) {
      console.error("controller has attached to view");
      return
    }
    this.view = view;
    this.jView.start();
  }

  detachFromView() {
    this.view = null;
    this.jView.stop();
  }

  setDelegate(delegate: SeatCanvasRendererDelegate | null) {
    if (delegate) {
      this.delegate = new WeakRef(delegate);

      this.jView.setDidLoadBaseMapCallback(delegate.didLoadBaseMap ? this.onDidLoadBaseMap : null);
      this.jView.setDidUnloadBaseMapCallback(delegate.didUnloadBaseMap ? this.onDidUnloadBaseMap : null);
      this.jView.setDidUpdateZoomLevelConfigCallback(delegate.didUpdateZoomLevelConfig ?
        this.onDidUpdateZoomLevelConfig :
        null);

      this.jView.setDidTapSeatCallback(this.onDidTapSeat);
      if (delegate.didTapZone) {
        this.jView.setDidTapZoneCallback(this.onDidTapZone);
      } else {
        this.jView.setDidTapZoneCallback(null);
      }

      this.jView.setViewportWillBeginDraggingCallback(delegate.willBeginDragging ? this.onViewportWillBeginDragging :
        null);
      this.jView.setViewportDidScrollCallback(delegate.didScroll ? this.onViewportDidScroll : null);
      this.jView.setViewportDidEndDraggingCallback(delegate.didEndDragging ? this.onViewportDidEndDragging : null);
      this.jView.setViewportDidEndDeceleratingCallback(delegate.didEndDecelerating ? this.onViewportDidEndDecelerating :
        null);
      this.jView.setViewportWillBeginZoomingCallback(delegate.willBeginZooming ? this.onViewportWillBeginZooming :
        null);
      this.jView.setViewportDidZoomCallback(delegate.didZoom ? this.onViewportDidZoom : null);
      this.jView.setViewportDidEndZoomingCallback(delegate.didEndZooming ? this.onViewportDidEndZooming : null);
      this.jView.setViewportDidEndScrollingAnimationCallback(delegate.didEndScrollingAnimation ?
        this.onViewportDidEndScrollingAnimation : null);
    } else {
      this.delegate = null;

      this.jView.setDidLoadBaseMapCallback(null);
      this.jView.setDidUnloadBaseMapCallback(null);
      this.jView.setDidUpdateZoomLevelConfigCallback(null);

      this.jView.setDidTapSeatCallback(null);
      this.jView.setDidTapZoneCallback(null);

      this.jView.setViewportWillBeginDraggingCallback(null);
      this.jView.setViewportDidScrollCallback(null);
      this.jView.setViewportDidEndDraggingCallback(null);
      this.jView.setViewportDidEndDeceleratingCallback(null);
      this.jView.setViewportWillBeginZoomingCallback(null);
      this.jView.setViewportDidZoomCallback(null);
      this.jView.setViewportDidEndZoomingCallback(null);
      this.jView.setViewportDidEndScrollingAnimationCallback(null);
    }
  }

  /**
   * 从 Assets 中加载底图
   * @param manager 资源管理器
   * @param name 资源名称
   * @param format 格式
   */
  async loadFromAssets(manager: resourceManager.ResourceManager, name: string, format: BaseMapFormat,
    parseConfig: BaseMapParseConfig | null = null) {
    let nativePtr = await seatcanvas.JRendererCore.ParseBaseMapFromAssets(manager, name, format,
      parseConfig?.serializeToString() ?? null)
    this.jView.loadBaseMap(nativePtr);
  }

  /**
   * 当前实例唯一标识符，用于绑定 XComponent
   * @returns 唯一标识符
   */
  uniqueID() {
    return this.jView.uniqueID();
  }

  /**
   * 启动渲染loop
   */
  start() {
    this.jView.start();
  }

  /**
   * 停止渲染loop
   */
  stop() {
    this.jView.stop();
  }

  /**
   * 画布背景色（ColorInt，AARRGGBB）。如 0xFFFFFFFF、0xfff5f5f5 等。
   */
  get canvasColor(): number {
    return this.jView.getCanvasColor();
  }

  set canvasColor(value: number) {
    this.jView.setCanvasColor(value);
  }

  /**
   * 座位大小
   */
  get seatSize(): number {
    return this.jView.getSeatSize();
  }

  set seatSize(value: number) {
    this.jView.setSeatSize(value);
  }

  /**
   * 当前内容允许的最小缩放比例
   */
  get minimumZoomScale(): number {
    return this.jView.getMinimumZoomScale();
  }

  /**
   * 当前内容允许的最大缩放比例
   */
  get maximumZoomScale(): number {
    return this.jView.getMaximumZoomScale();
  }

  /**
   * 当前缩放级别
   */
  get zoomScale(): number {
    return this.jView.getZoomScale();
  }

  /**
   * 获取当前显示范围（原始坐标系）
   */
  visibleOriginalRect(): seatcanvas.Rect {
    return this.jView.getVisibleOriginalRect();
  }

  /**
   * 查找与指定矩形相交的区域 ID 列表（原始坐标系）
   * @param rect 查询矩形，通常配合 visibleOriginalRect() 使用
   */
  zoneIdsInOriginalRect(rect: seatcanvas.Rect): string[] {
    return this.jView.getZoneIdsInOriginalRect(rect);
  }

  get zoomLevel(): ZoomLevel {
    return this.jView.zoomLevel()
  }

  /**
   * 座位渲染阈值（控制从彩虹图切换到绘制座位的缩放级别）
   * 默认等同于内部计算得到的 ZoomLevelConfig.venue
   */
  get seatRenderZoomThreshold(): number {
    return this.jView.getSeatRenderZoomThreshold();
  }

  set seatRenderZoomThreshold(value: number) {
    this.jView.setSeatRenderZoomThreshold(value);
  }

  /**
   * 是否绘制调试 HUD（FPS、缩放级别、座位统计等），默认关闭
   */
  get debugHUDEnabled(): boolean {
    return this.jView.isDebugHUDEnabled();
  }

  set debugHUDEnabled(value: boolean) {
    this.jView.setDebugHUDEnabled(value);
  }

  /**
   * 释放内部 C++ 资源， 调用后将不能在继续使用当前对象
   */
  release() {
    this.jView.release();
  }

  /**
   * 处理点击手势
   * @param x X 方向移动量（px单位）
   * @param y Y 方向移动量（px单位）
   */
  handleTap(x: number, y: number) {
    this.jView.handleTap(x, y);
  }

  /**
   * 处理滑动手势
   * @param state 手势状态
   * @param tx X 方向移动量
   * @param ty Y 方向移动量
   */
  handlePan(state: seatcanvas.GestureState, tx: number, ty: number) {
    this.jView.handlePan(state, tx, ty);
  }

  /**
   * 处理缩放手势
   * @param state 手势状态
   * @param scale 缩放比
   * @param cx 缩放 X 中心
   * @param cy 缩放 Y 中心
   */
  handlePinch(state: seatcanvas.GestureState, scale: number, cx: number, cy: number) {
    this.jView.handlePinch(state, scale, cx, cy);
  }

  /**
   * 缩放到指定区域
   * @param bounds 原始内容的区域
   * @param animated 是否需要动画
   * @param padding 区域周围的边距（在内容坐标系中），默认为 0
   * @param duration 动画持续时间（毫秒），仅在 animated 为 true 时有效，默认 300ms
   */
  zoomToRect(bounds: seatcanvas.Rect, animated: boolean = true, padding: number = 0, duration: number = 300) {
    this.jView.zoomToRect(bounds, animated, padding, duration)
  }

  /**
   * 应用样式配置（使用 SeatStyleConfigBuilder 构建）
   * @param config JSON 样式数据
   */
  applySeatStyleJSONConfig(config: string | null) {
    this.jView.applySeatStyleJSONConfig(config);
  }

  /**
   * 更新区域颜色数据
   * @param colors 区域颜色表
   */
  updateSeatZoneAlternateColors(colors: SeatZoneColor[]) {
    this.jView.updateSeatZoneAlternateColors(colors);
  }

  /**
   * 更新小地图区域颜色数据
   * @param colors 区域颜色表
   */
  updateMiniMapZoneAlternateColors(colors: SeatZoneColor[]) {
    this.jView.updateMiniMapZoneAlternateColors(colors);
  }

  /**
   * 更新指定区域下的座位数据。
   * 会重置该 zone 的 status 为 0，并清除旧 seat 的 selected；随后需 re-push status/selected。
   * @param zoneId 区域 ID
   * @param seats 座位数据列表
   */
  updateSeats(zoneId: string, seats: SeatData[]) {
    this.jView.updateSeats(zoneId, seats);
  }

  /** 注册价档表（load 前调用一次） */
  registerPricecodes(pricecodes: string[]) {
    this.jView.registerPricecodes(pricecodes);
  }

  /** 批量更新单个座位 status */
  updateSeatStatuses(updates: SeatStatusUpdate[]) {
    this.jView.updateSeatStatuses(updates);
  }

  /** 批量更新某个 zone 内全部座位 status（数组下标与 updateSeats 顺序一致） */
  updateSeatStatusesForZone(zoneId: string, statuses: number[]) {
    this.jView.updateSeatStatusesForZone(zoneId, statuses);
  }

  /** 全量替换选中座位 */
  setSelectedSeatIds(seatIds: string[]) {
    this.jView.setSelectedSeatIds(seatIds);
  }

  /** 增量更新选中座位 */
  updateSelectedSeatIds(added: string[], removed: string[]) {
    this.jView.updateSelectedSeatIds(added, removed);
  }

  /**
   * 清除全部座位数据
   */
  clearSeatData() {
    this.jView.clearSeatData();
  }

  readonly onDidLoadBaseMap = () => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didLoadBaseMap) {
      delegate.didLoadBaseMap();
    }
  }
  readonly onDidUnloadBaseMap = () => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didUnloadBaseMap) {
      delegate.didUnloadBaseMap();
    }
  }
  readonly onDidUpdateZoomLevelConfig = (event: SeatCanvasZoomLevelConfigEvent) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didUpdateZoomLevelConfig) {
      delegate.didUpdateZoomLevelConfig(event);
    }
  }
  readonly onDidTapSeat = (zoneId: string, seatId: string): boolean => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didTapSeat) {
      return delegate.didTapSeat(zoneId, seatId);
    }
    return false;
  }
  readonly onDidTapZone = (zoneId: string) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didTapZone) {
      delegate.didTapZone(zoneId);
    }
  }
  readonly onViewportWillBeginDragging = (viewport: SeatCanvasViewport) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.willBeginDragging) {
      delegate.willBeginDragging(viewport);
    }
  }
  readonly onViewportDidScroll = (viewport: SeatCanvasViewport) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didScroll) {
      delegate.didScroll(viewport);
    }
  }
  readonly onViewportDidEndDragging = (viewport: SeatCanvasViewport, willDecelerate: boolean) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didEndDragging) {
      delegate.didEndDragging(viewport, willDecelerate);
    }
  }
  readonly onViewportDidEndDecelerating = (viewport: SeatCanvasViewport) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didEndDecelerating) {
      delegate.didEndDecelerating(viewport);
    }
  }
  readonly onViewportWillBeginZooming = (viewport: SeatCanvasViewport) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.willBeginZooming) {
      delegate.willBeginZooming(viewport);
    }
  }
  readonly onViewportDidZoom = (viewport: SeatCanvasViewport) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didZoom) {
      delegate.didZoom(viewport);
    }
  }
  readonly onViewportDidEndZooming = (viewport: SeatCanvasViewport) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didEndZooming) {
      delegate.didEndZooming(viewport);
    }
  }
  readonly onViewportDidEndScrollingAnimation = (viewport: SeatCanvasViewport) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didEndScrollingAnimation) {
      delegate.didEndScrollingAnimation(viewport);
    }
  }
}
