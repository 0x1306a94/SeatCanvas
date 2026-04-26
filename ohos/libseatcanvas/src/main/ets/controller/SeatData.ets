/**
 * 单个座位的几何与标识数据；业务状态由 styleIdForSeat 返回的样式 ID 表示。
 */
export interface SeatData {
  seatId: string;
  x: number;
  y: number;
  /** 旋转角（度，顺时针），缺省为 0 */
  rotation?: number;
}
