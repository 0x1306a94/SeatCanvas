import { SeatStyleConfig } from './SeatStyleConfig';
import { SeatStyleType } from './SeatStyleType';

export class CircleSeatStyleConfig implements SeatStyleConfig {
  type: SeatStyleType = SeatStyleType.CIRCLE;
  fill: string;
  overlay?: string;
  checkmark?: string;

  constructor(fill: string, overlay?: string, checkmark?: string) {
    this.fill = fill;
    this.overlay = overlay;
    this.checkmark = checkmark;
  }
}
