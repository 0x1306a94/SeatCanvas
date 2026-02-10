import { seatcanvas } from 'libseatcanvas.so';
import { font } from '@kit.ArkUI';
import { deviceInfo } from '@kit.BasicServicesKit';
import { text } from '@kit.ArkGraphics2D';

export class SeatCanvasInit {
  private static hasInit: boolean = false;

  static Init() {
    if (SeatCanvasInit.hasInit) {
      return;
    }
    SeatCanvasInit.InitFontFallback();
    SeatCanvasInit.hasInit = true;
  }

  private static InitFontFallback() {
    // API14+: prefer the new async API, fallback to the legacy API on failure
    if (deviceInfo.sdkApiVersion >= 14) {
      SeatCanvasInit.InitFontFallbackWithTextModule().then((success: boolean) => {
        if (!success) {
          SeatCanvasInit.InitFontFallbackWithFontModule();
        }
      });
      return;
    }

    // Below API14: use the legacy API
    SeatCanvasInit.InitFontFallbackWithFontModule();
  }

  // Use the new text module API (API14+)
  private static async InitFontFallbackWithTextModule(): Promise<boolean> {
    try {
      let fontNames: string[] = await text.getSystemFontFullNamesByType(text.SystemFontType.ALL);
      if (fontNames && fontNames.length > 0) {
        let descriptors = await Promise.all(fontNames.map(fontName =>
        text.getFontDescriptorByFullName(fontName, text.SystemFontType.ALL)
        ));
        let paths = Array.from(new Set(descriptors.filter(d => d && d.path).map(d => d.path)));
        if (paths.length > 0) {
          seatcanvas.JFont.SetFallbackFontPaths(paths);
          return true;
        }
      }
    } catch (e) {
      console.warn('[SeatCanvasInit] text.getSystemFontFullNamesByType failed: ' + e);
    }
    return false;
  }

  private static async InitFontFallbackWithFontModule() {
    let fontConfig = font.getUIFontConfig();
    let paths = new Array<string>();
    let emojiPaths = new Array<string>();
    for (let index = 0; index < fontConfig.fallbackGroups.length; index++) {
      const element = fontConfig.fallbackGroups[index];
      for (let j = 0; j < element.fallback.length; j++) {
        let fontInfo = font.getFontByName(element.fallback[j].family)
        if (fontInfo == null) {
          fontInfo = font.getFontByName(element.fallback[j].family + " Regular")
        }
        if (fontInfo != null) {
          if (fontInfo.fullName.toLowerCase().indexOf("emoji") > 0) {
            emojiPaths.push(fontInfo.path);
          } else {
            if (fontInfo.path.indexOf("HarmonyOS_Sans") >= 0) {
              paths.unshift(fontInfo.path); // 将 HarmonyOS_Sans 的字体路径移动到最前面
            } else {
              paths.push(fontInfo.path);
            }
          }
        }
      }
    }
    paths.concat(emojiPaths);
    seatcanvas.JFont.SetFallbackFontPaths(paths);
  }
}