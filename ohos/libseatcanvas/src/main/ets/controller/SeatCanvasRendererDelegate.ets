import { seatcanvas } from 'libseatcanvas.so';
import { SeatCanvasZoomLevelConfigEvent } from './SeatCanvasZoomLevelConfigEvent';

export interface SeatCanvasViewport {
  zoomScale: number;
  contentOffsetX: number;
  contentOffsetY: number;
  visibleOriginalRect: seatcanvas.Rect;
}

/**
 * 座位渲染器代理：点击与视口事件由核心层驱动回调；座位样式通过推送 API 更新
 */
export interface SeatCanvasRendererDelegate {
  /**
   * 底图加载完成；push 座位数据和样式配置
   */
  didLoadBaseMap?: () => void;

  /**
   * 底图已卸载
   */
  didUnloadBaseMap?: () => void;

  /**
   * 缩放级别配置已更新（底图加载完成或设备旋转后触发）；可在此设置 seatRenderZoomThreshold
   */
  didUpdateZoomLevelConfig?: (event: SeatCanvasZoomLevelConfigEvent) => void;

  /**
   * 点击座位；状态变化且需要重绘时返回 true
   */
  didTapSeat?: (zoneId: string, seatId: string) => boolean;

  /**
   * 点击区域
   */
  didTapZone?: (zoneId: string) => void;

  /**
   * 即将开始拖动视图
   */
  willBeginDragging?: (viewport: SeatCanvasViewport) => void;

  /**
   * 视图发生滚动
   */
  didScroll?: (viewport: SeatCanvasViewport) => void;

  /**
   * 拖动手势结束
   */
  didEndDragging?: (viewport: SeatCanvasViewport, decelerate: boolean) => void;

  /**
   * 惯性滚动或回弹动画结束
   */
  didEndDecelerating?: (viewport: SeatCanvasViewport) => void;

  /**
   * 即将开始缩放视图
   */
  willBeginZooming?: (viewport: SeatCanvasViewport) => void;

  /**
   * 视图发生缩放
   */
  didZoom?: (viewport: SeatCanvasViewport) => void;

  /**
   * 缩放手势结束
   */
  didEndZooming?: (viewport: SeatCanvasViewport) => void;

  /**
   * 程序触发的滚动或缩放动画结束
   */
  didEndScrollingAnimation?: (viewport: SeatCanvasViewport) => void;

}
