import { ZoomLevel } from './ZoomLevel';
import { seatcanvas } from 'libseatcanvas.so';

/**
 * Base map loaded event, mirrors C++ SeatCanvasBaseMapLoadedEvent.
 */
export interface SeatCanvasBaseMapLoadedEvent {
  baseMapWidth: number;
  baseMapHeight: number;
  zoomLevels: ZoomLevel;
  minimumZoomScale: number;
  maximumZoomScale: number;
  zoomScale: number;
  visibleOriginalRect: seatcanvas.Rect;
}
