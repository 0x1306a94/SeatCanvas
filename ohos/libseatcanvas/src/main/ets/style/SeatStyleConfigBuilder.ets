import { SeatStyleKey } from './SeatStyleKey';
import { SeatStyleConfig } from './SeatStyleConfig';
import { CircleSeatStyleConfig } from './CircleSeatStyleConfig';
import { SVGSeatStyleConfig } from './SVGSeatStyleConfig';

interface StyleConfigEntry {
  key: SeatStyleKey
  config: SeatStyleConfig
}

export class SeatStyleConfigBuilder {
  private configs: Map<string, SeatStyleConfig> = new Map();

  private getKeyString(key: SeatStyleKey): string {
    return `${key.status}_${key.selected}`;
  }

  /**
   * 添加圆形样式配置
   * 颜色参数为 #AARRGGBB 或者 #RRGGBB 格式的 hex 字符串
   * @param status 座位状态
   * @param selected 是否选中
   * @param fill 填充颜色
   * @param overlay 覆盖层颜色（选中时显示）
   * @param checkmark 勾选标记颜色
   * @returns Builder 实例，支持链式调用
   */
  addCircleStyle(
    status: number,
    selected: boolean,
    fill: string,
    overlay: string,
    checkmark: string
  ): SeatStyleConfigBuilder {
    const key = new SeatStyleKey(status, selected);
    const config = new CircleSeatStyleConfig(fill, overlay, checkmark);
    this.configs.set(this.getKeyString(key), config);
    return this;
  }

  /**
   * 添加 SVG 样式配置
   * @param status 座位状态
   * @param selected 是否选中
   * @param content SVG 内容
   * @returns Builder 实例，支持链式调用
   */
  addSVGStyle(
    status: number,
    selected: boolean,
    content: string
  ): SeatStyleConfigBuilder {
    const key = new SeatStyleKey(status, selected);
    const config = new SVGSeatStyleConfig(content);
    this.configs.set(this.getKeyString(key), config);
    return this;
  }


  /**
   * 序列化为 JSON 字符串
   * @returns JSON 字符串，如果序列化失败则返回 null
   */
  toJSONString(): string | null {
    try {
      const entries: StyleConfigEntry[] = [];

      this.configs.forEach((config, key) => {
        const parts = key.split('_');
        const status = parseInt(parts[0]);
        const selected = parts[1] === 'true';

        const entry: StyleConfigEntry = {
          key: new SeatStyleKey(status, selected),
          config: config,
        };
        entries.push(entry);
      });
      return JSON.stringify(entries, null, 4);
    } catch (e) {
      return null;
    }
  }

  /**
   * 清空所有配置
   */
  clear(): void {
    this.configs.clear();
  }
}
