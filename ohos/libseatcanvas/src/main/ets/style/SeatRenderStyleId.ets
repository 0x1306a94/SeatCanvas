import { seatcanvas } from 'libseatcanvas.so';

/**
 * 生成与 C++ 渲染器相同格式的座位样式 ID
 */
export class SeatRenderStyleId {
  /**
   * @param pricecode 价档 code；null/undefined/空字符串表示无价档槽位
   * @param status 业务自定义座位状态
   * @param selected 是否选中
   */
  static compose(pricecode: string | null | undefined, status: number, selected: boolean): string {
    return seatcanvas.JSeatRenderStyleId.compose(pricecode ?? '', status, selected);
  }
}
