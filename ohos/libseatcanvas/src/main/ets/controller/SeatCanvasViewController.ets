import { seatcanvas } from 'libseatcanvas.so';
import { resourceManager } from '@kit.LocalizationKit';
import { display } from '@kit.ArkUI';
import { SeatCanvasInit } from '../utils/SeatCanvasInit';
import { BaseMapFormat } from '../utils/BaseMapFormat';
import { SeatCanvasRendererDelegate } from './SeatCanvasRendererDelegate';
import { SeatData } from './SeatData';
import { SeatZoneData } from './SeatZoneData';

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
      // 设置 native 回调
      if (delegate.shouldSelectSeat) {
        this.jView.setShouldSelectSeatCallback(this.onShouldSelectSeat);
      } else {
        this.jView.setShouldSelectSeatCallback(null);
      }
      if (delegate.didSelectSeat) {
        this.jView.setDidSelectSeatCallback(this.onDidSelectSeat);
      } else {
        this.jView.setDidSelectSeatCallback(null);
      }
      if (delegate.didDeselectSeat) {
        this.jView.setDidDeselectSeatCallback(this.onDidDeselectSeat);
      } else {
        this.jView.setDidDeselectSeatCallback(null);
      }
    } else {
      this.delegate = null;
      // 清除 native 回调
      this.jView.setShouldSelectSeatCallback(null);
      this.jView.setDidSelectSeatCallback(null);
      this.jView.setDidDeselectSeatCallback(null);
    }
  }

  /**
   * 从 Assets 中加载底图
   * @param manager 资源管理器
   * @param name 资源名称
   * @param format 格式
   */
  async loadFromAssets(manager: resourceManager.ResourceManager, name: string, format: BaseMapFormat) {
    let nativePtr = await seatcanvas.JRendererCore.ParseBaseMapFromAssets(manager, name, format)
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
   * 高亮指定区域：高亮区域 additionalAlpha=1.0，其余为 nonHighlightedAlpha
   * @param zoneIds 高亮区域的 ID 集合
   * @param nonHighlightedAlpha 非高亮区域的 additionalAlpha
   */
  setHighlightedZoneIds(zoneIds: string[], nonHighlightedAlpha: number): void {
    this.jView.setHighlightedZoneIds(zoneIds, nonHighlightedAlpha);
  }

  /**
   * 清除高亮，将所有区域 additionalAlpha 重置为 1.0
   */
  clearHighlightedZones(): void {
    this.jView.clearHighlightedZones();
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
   * 更新区域（zone）样式数据
   * @param zones 区域数据列表
   */
  updateSeatZones(zones: SeatZoneData[]) {
    this.jView.updateSeatZones(zones);
  }

  /**
   * 更新指定区域下的座位数据
   * @param zoneId 区域 ID
   * @param seats 座位数据列表
   */
  updateSeats(zoneId: string, seats: SeatData[]) {
    this.jView.updateSeats(zoneId, seats);
  }

  readonly onShouldSelectSeat = (zoneId: string, seatId: string): boolean => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.shouldSelectSeat) {
      return delegate.shouldSelectSeat(zoneId, seatId);
    }
    return false
  }
  readonly onDidSelectSeat = (zoneId: string, seatId: string) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didSelectSeat) {
      delegate.didSelectSeat(zoneId, seatId);
    }
  }
  readonly onDidDeselectSeat = (zoneId: string, seatId: string) => {
    const delegate = this.delegate?.deref();
    if (delegate && delegate.didDeselectSeat) {
      delegate.didDeselectSeat(zoneId, seatId);
    }
  }
}