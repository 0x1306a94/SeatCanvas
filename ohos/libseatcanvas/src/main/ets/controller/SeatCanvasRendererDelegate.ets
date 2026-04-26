/**
 * 座位渲染器代理：样式 ID 与点击由核心层驱动回调。
 */
export interface SeatCanvasRendererDelegate {
  /**
   * 根据区域与座位返回样式 ID；返回 null/undefined 表示不绘制该座位。
   */
  styleIdForSeat?: (zoneId: string, seatId: string) => string | null;

  /**
   * 点击座位；状态变化且需要重绘时返回 true。
   */
  didTapSeat?: (zoneId: string, seatId: string) => boolean;

  /**
   * 点击区域（非座位区域）。
   */
  didTapZone?: (zoneId: string) => void;
}
