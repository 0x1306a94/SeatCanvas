export interface SeatCanvasRendererDelegate {
  /**
   * 是否可以选中座位
   * @param regionId 区域ID
   * @param seatId 座位ID
   * @return true 表示可以选中，false 表示不能选中
   */
  shouldSelectSeat?: (regionId: string, seatId: string) => boolean;

  /**
   * 选中某个座位
   * @param regionId 区域ID
   * @param seatId 座位ID
   */
  didSelectSeat?: (regionId: string, seatId: string) => void;

  /**
   * 取消选中某个座位
   * @param regionId 区域ID
   * @param seatId 座位ID
   */
  didDeselectSeat?: (regionId: string, seatId: string) => void;
}