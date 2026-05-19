export class ColorExtensions {
  static rgbHexFromArgb(argbHex: string): string | null {
    if (!argbHex || !argbHex.startsWith('#')) {
      return null;
    }
    const hex = argbHex.substring(1);
    if (hex.length !== 8) {
      return null;
    }
    const r = hex.substring(2, 4);
    const g = hex.substring(4, 6);
    const b = hex.substring(6, 8);
    return `#${r}${g}${b}`.toUpperCase();
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
