import { SeatStyleConfigBuilder } from 'libseatcanvas'
import { resourceManager } from '@kit.LocalizationKit'
import { util } from '@kit.ArkTS';
import { ColorExtensions } from './ColorExtensions';
import { MockPriceData } from './MockPriceData';

export class SeatStyleBuilder {
  private static readonly SEAT_DISABLED_COLOR: string = '#FFDDDDDD';

  private static styleId(pricecode: string, available: boolean, selected: boolean): string {
    return `pricecode_${pricecode}_available_${available}_selected_${selected ? 1 : 0}`;
  }

  static buildCircleSeatStyleConfig(prices: MockPriceData[]): string | null {
    if (prices.length === 0) {
      return null;
    }
    const builder = new SeatStyleConfigBuilder();
    const disabled = SeatStyleBuilder.SEAT_DISABLED_COLOR;
    const overlay = '#B2000000';
    const checkmark = '#FFFFFFFF';
    for (const price of prices) {
      builder.addCircleStyle(SeatStyleBuilder.styleId(price.code, false, false), disabled);
      builder.addCircleStyle(SeatStyleBuilder.styleId(price.code, true, false), price.color);
      builder.addCircleStyle(SeatStyleBuilder.styleId(price.code, true, true), price.color, overlay, checkmark);
    }
    return builder.toJSONString();
  }

  static buildSVGSeatStyleConfig(
    manager: resourceManager.ResourceManager,
    prices: MockPriceData[]
  ): string | null {
    if (prices.length === 0) {
      return null;
    }

    const available = SeatStyleBuilder.loadSVGContent(manager, 'icon_seat_selectable.svg');
    const selected = SeatStyleBuilder.loadSVGContent(manager, 'icon_seat_selected.svg');
    const disabled = SeatStyleBuilder.loadSVGContent(manager, 'icon_seat_nonselectable.svg');

    if (available == null || selected == null || disabled == null) {
      return null;
    }

    const builder = new SeatStyleConfigBuilder();
    for (const price of prices) {
      builder.addSVGStyle(SeatStyleBuilder.styleId(price.code, false, false), disabled);
      const rgbHex = ColorExtensions.rgbHexFromArgb(price.color);
      if (rgbHex == null) {
        continue;
      }
      const modifiedAvailable = available.replace(/#EB484A/gi, rgbHex);
      const modifiedSelected = selected.replace(/#5BC64D/gi, rgbHex);
      builder.addSVGStyle(SeatStyleBuilder.styleId(price.code, true, false), modifiedAvailable);
      builder.addSVGStyle(SeatStyleBuilder.styleId(price.code, true, true), modifiedSelected);
    }
    return builder.toJSONString();
  }

  private static loadSVGContent(manager: resourceManager.ResourceManager, name: string): string | null {
    try {
      const fileName = `SeatCanvasSampleAssets/default/seatstyle/${name}`;
      const rawFileContent = manager.getRawFileContentSync(fileName);
      if (rawFileContent == null) {
        console.error(`Failed to load SVG file: ${fileName}`);
        return null;
      }
      const textDecoderOptions: util.TextDecoderOptions = {
        fatal: false,
        ignoreBOM: true
      };
      const decodeToStringOptions: util.DecodeToStringOptions = {
        stream: false
      };
      const textDecoder = util.TextDecoder.create('utf-8', textDecoderOptions);
      return textDecoder.decodeToString(rawFileContent, decodeToStringOptions);
    } catch (e) {
      console.error(`Error loading SVG file ${name}:`, e);
      return null;
    }
  }
}
