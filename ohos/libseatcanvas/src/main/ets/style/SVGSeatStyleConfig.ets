import { SeatStyleConfig } from './SeatStyleConfig';
import { SeatStyleType } from './SeatStyleType';

export class SVGSeatStyleConfig implements SeatStyleConfig {
  type: SeatStyleType = SeatStyleType.SVG;
  content: string;

  constructor(content: string) {
    this.content = content;
  }
}
