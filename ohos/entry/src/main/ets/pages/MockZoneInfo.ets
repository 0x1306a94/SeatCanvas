import { ColorExtensions } from './ColorExtensions';

export interface MockZoneInfo {
  zoneId: string;
  x: number;
  y: number;
  w: number;
  h: number;
  alternateColor?: string;
}

export class MockZoneInfoHelper {
  static getAlternateColor(zone: MockZoneInfo): string | undefined {
    return zone.alternateColor ? ColorExtensions.colorFromArgbHex(zone.alternateColor) || undefined : undefined;
  }
}
