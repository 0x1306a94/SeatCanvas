import { SeatStyleConfig } from './SeatStyleConfig';
import { CircleSeatStyleConfig } from './CircleSeatStyleConfig';
import { SVGSeatStyleConfig } from './SVGSeatStyleConfig';

/**
 * 构建与核心层一致的样式 JSON：顶层为数组，元素含 `key` 与 `config`
 */
export class SeatStyleConfigBuilder {
  private configs: Map<string, SeatStyleConfig> = new Map();

  /**
   * 注册圆形样式
   * @param styleId 样式键（与 SeatRenderStyleId.compose 返回值一致）
   * @param fill 填充色（#AARRGGBB 或 #RRGGBB）
   * @param overlay 遮罩色，仅选中态需要时传入
   * @param checkmark 对勾色，仅选中态需要时传入
   */
  addCircleStyle(styleId: string, fill: string, overlay?: string, checkmark?: string): SeatStyleConfigBuilder {
    if (!styleId || styleId.length === 0) {
      return this;
    }
    this.configs.set(styleId, new CircleSeatStyleConfig(fill, overlay, checkmark));
    return this;
  }

  /**
   * 注册 SVG 样式
   */
  addSVGStyle(styleId: string, content: string): SeatStyleConfigBuilder {
    if (!styleId || styleId.length === 0) {
      return this;
    }
    this.configs.set(styleId, new SVGSeatStyleConfig(content));
    return this;
  }

  toJSONString(): string | null {
    try {
      const arr: ESObject[] = [];
      this.configs.forEach((config, styleId) => {
        arr.push({
          key: styleId,
          config: SeatStyleConfigBuilder.configToPlainObject(config)
        });
      });
      return JSON.stringify(arr);
    } catch (e) {
      return null;
    }
  }

  private static configToPlainObject(config: SeatStyleConfig): Record<string, string | number> {
    if (config instanceof CircleSeatStyleConfig) {
      const o: Record<string, string | number> = {};
      o['type'] = config.type;
      o['fill'] = config.fill;
      if (config.overlay !== undefined) {
        o['overlay'] = config.overlay;
      }
      if (config.checkmark !== undefined) {
        o['checkmark'] = config.checkmark;
      }
      return o;
    }
    if (config instanceof SVGSeatStyleConfig) {
      const o: Record<string, string | number> = {};
      o['type'] = config.type;
      o['content'] = config.content;
      return o;
    }
    return {};
  }

  clear(): void {
    this.configs.clear();
  }
}
