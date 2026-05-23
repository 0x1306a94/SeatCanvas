import { ZoomLevel } from './ZoomLevel';

/**
 * 缩放级别配置更新事件。
 */
export interface SeatCanvasZoomLevelConfigEvent {
  zoomLevels: ZoomLevel;
  minimumZoomScale: number;
  maximumZoomScale: number;
  zoomScale: number;
}
