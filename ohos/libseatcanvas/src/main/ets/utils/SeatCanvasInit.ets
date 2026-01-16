import { seatcanvas } from 'libseatcanvas.so';
import { font } from '@kit.ArkUI';

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