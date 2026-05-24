/**
 * 单个座位的几何与标识数据；可变状态通过 status / selected API 推送
 */
export interface SeatData {
  /** 座位ID */
  seatId: string;

  /** 座位X （左上角）*/
  x: number;

  /** 座位Y （左上角）*/
  y: number;

  /** 旋转角（度，顺时针），缺省为 0 */
  rotation?: number;

  /** 价档 code，缺省或空表示无价档 */
  pricecode?: string;
}

export interface SeatStatusUpdate {
  /** 座位 ID */
  seatId: string;

  /** 业务自定义座位状态 */
  status: number;
}
