import { ColorExtensions } from './ColorExtensions';

export interface MockZoneInfo {
  zoneId: string;
  x: number;
  y: number;
  w: number;
  h: number;
  color?: string;
  priceColor?: string;
}

export class MockZoneInfoHelper {
  static getColor(zone: MockZoneInfo): string | undefined {
    return zone.color ? ColorExtensions.colorFromArgbHex(zone.color) || undefined : undefined;
  }

  static getPriceColor(zone: MockZoneInfo): string | undefined {
    return zone.priceColor ? ColorExtensions.colorFromArgbHex(zone.priceColor) || undefined : undefined;
  }
}
