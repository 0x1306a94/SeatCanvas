export class ColorExtensions {
  static rgbaHex(color: string): string {
    // OHOS 使用 ARGB 格式的颜色字符串，例如 "#FFFFFFFF"
    // 这里假设输入已经是正确的格式，如果需要转换可以添加逻辑
    return color;
  }

  static argbHex(color: string): string {
    // OHOS 使用 ARGB 格式
    return color;
  }

  static colorFromArgbHex(argbHex: string): string | null {
    // 验证并返回 ARGB hex 字符串
    if (!argbHex || !argbHex.startsWith('#')) {
      return null;
    }
    const hex = argbHex.substring(1);
    if (hex.length !== 8) {
      return null;
    }
    // 验证是否为有效的十六进制
    if (!/^[0-9A-Fa-f]{8}$/.test(hex)) {
      return null;
    }
    return argbHex;
  }
}
