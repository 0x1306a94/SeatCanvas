import { ColorExtensions } from './ColorExtensions';

export interface MockZoneInfo {
  zoneId: string;
  x: number;
  y: number;
  w: number;
  h: number;
  color?: string;
  rainbowColor?: string;
}

export class MockZoneInfoHelper {
  static getColor(zone: MockZoneInfo): string | undefined {
    return zone.color ? ColorExtensions.colorFromArgbHex(zone.color) || undefined : undefined;
  }

  static getRainbowColor(zone: MockZoneInfo): string | undefined {
    return zone.rainbowColor ? ColorExtensions.colorFromArgbHex(zone.rainbowColor) || undefined : undefined;
  }
}
