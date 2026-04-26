import { SeatStyleConfigBuilder } from 'libseatcanvas'
import { resourceManager } from '@kit.LocalizationKit'
import { util } from '@kit.ArkTS';

export class SeatStyleBuilder {
  private static sampleStyleId(status: number, selected: boolean): string {
    return `status_${status}_selected_${selected ? 1 : 0}`;
  }

  static BuildCircleSeatStyleConfig(): string | null {
    let builder = new SeatStyleConfigBuilder();
    const overlay = '#B2000000';
    const checkmark = '#FFFFFFFF';
    const fills: string[] = ['#FFFF0000', '#FFAAAAAA', '#FF999999', '#FF666666'];
    for (let s = 0; s <= 3; s++) {
      builder.addCircleStyle(SeatStyleBuilder.sampleStyleId(s, false), fills[s]);
      builder.addCircleStyle(SeatStyleBuilder.sampleStyleId(s, true), fills[s], overlay, checkmark);
    }
    return builder.toJSONString();
  }

  /**
   * 构建 SVG 座位样式配置
   * @param manager 资源管理器，用于加载 SVG 文件
   * @returns JSON 字符串，如果构建失败则返回 null
   */
  static BuildSVGSeatStyleConfig(manager: resourceManager.ResourceManager): string | null {

    let available = SeatStyleBuilder.loadSVGContent(manager, 'icon_seat_selectable.svg');
    let selected = SeatStyleBuilder.loadSVGContent(manager, 'icon_seat_selected.svg');
    let disabled = SeatStyleBuilder.loadSVGContent(manager, 'icon_seat_nonselectable.svg');

    if (available == null || selected == null || disabled == null) {
      return null;
    }

    let builder = new SeatStyleConfigBuilder();

    builder.addSVGStyle(SeatStyleBuilder.sampleStyleId(0, false), available);
    builder.addSVGStyle(SeatStyleBuilder.sampleStyleId(0, true), selected);

    for (let s = 1; s <= 3; s++) {
      builder.addSVGStyle(SeatStyleBuilder.sampleStyleId(s, false), disabled);
      builder.addSVGStyle(SeatStyleBuilder.sampleStyleId(s, true), disabled);
    }

    return builder.toJSONString();
  }

  /**
   * 从资源文件加载 SVG 内容
   * @param manager 资源管理器
   * @param name SVG 文件名（不含扩展名）
   * @returns SVG 内容字符串，如果加载失败则返回 null
   */
  private static loadSVGContent(manager: resourceManager.ResourceManager, name: string): string | null {
    try {
      let fileName = `SeatCanvasSampleAssets/default/seatstyle/${name}`;
      let rawFileContent = manager.getRawFileContentSync(fileName);
      if (rawFileContent == null) {
        console.error(`Failed to load SVG file: ${fileName}`);
        return null;
      }
      let textDecoderOptions: util.TextDecoderOptions = {
        fatal: false,
        ignoreBOM: true
      }
      let decodeToStringOptions: util.DecodeToStringOptions = {
        stream: false
      }
      let textDecoder = util.TextDecoder.create('utf-8', textDecoderOptions);
      let retStr = textDecoder.decodeToString(rawFileContent, decodeToStringOptions);
      return retStr;
    } catch (e) {
      console.error(`Error loading SVG file ${name}:`, e);
      return null;
    }
  }
}
