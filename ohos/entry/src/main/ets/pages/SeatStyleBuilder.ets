import { SeatStyleConfigBuilder } from 'libseatcanvas'
import { resourceManager } from '@kit.LocalizationKit'
import { util } from '@kit.ArkTS';

export class SeatStyleBuilder {
  static BuildCircleSeatStyleConfig(): string | null {
    let builder = new SeatStyleConfigBuilder();
    builder.addCircleStyle(0, false, '#FFFF0000', '#B2000000', '#FFFFFFFF');
    builder.addCircleStyle(0, true, '#FFFF0000', '#B2000000', '#FFFFFFFF');
    builder.addCircleStyle(1, false, '#FFAAAAAA', '#B2000000', '#FFFFFFFF');
    builder.addCircleStyle(1, true, '#FFAAAAAA', '#B2000000', '#FFFFFFFF');
    builder.addCircleStyle(2, false, '#FF999999', '#B2000000', '#FFFFFFFF');
    builder.addCircleStyle(2, true, '#FF999999', '#B2000000', '#FFFFFFFF');
    builder.addCircleStyle(3, false, '#FF666666', '#B2000000', '#FFFFFFFF');
    builder.addCircleStyle(3, true, '#FF666666', '#B2000000', '#FFFFFFFF');
    return builder.toJSONString();
  }

  /**
   * 构建 SVG 座位样式配置
   * @param manager 资源管理器，用于加载 SVG 文件
   * @returns JSON 字符串，如果构建失败则返回 null
   */
  static BuildSVGSeatStyleConfig(manager: resourceManager.ResourceManager): string | null {

    let available = SeatStyleBuilder.loadSVGContent(manager, 'icon_seat_selectable');
    let selected = SeatStyleBuilder.loadSVGContent(manager, 'icon_seat_selected');
    let disabled = SeatStyleBuilder.loadSVGContent(manager, 'icon_seat_nonselectable');

    if (available == null || selected == null || disabled == null) {
      return null;
    }

    let builder = new SeatStyleConfigBuilder();

    builder.addSVGStyle(0, false, available);
    builder.addSVGStyle(0, true, selected);

    builder.addSVGStyle(1, false, disabled);
    builder.addSVGStyle(1, true, disabled);

    builder.addSVGStyle(2, false, disabled);
    builder.addSVGStyle(2, true, disabled);

    builder.addSVGStyle(3, false, disabled);
    builder.addSVGStyle(3, true, disabled);

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
      let fileName = `svg/${name}.svg`;
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