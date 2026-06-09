//
//  SeatCanvasFont.ts
//  SeatCanvas
//
//  Created by king on 2026/5/27.
//

import { getSeatCanvasModule } from './SeatCanvas-module';

export class SeatCanvasFont {
    /**
     * 注册 fallback 字体字节数据。
     *
     * 调用顺序：`SeatCanvasInit()` 完成之后、`app.init()` 创建渲染器之前。
     * 字体文件的 fetch/加载由使用方负责；npm 包不包含字体文件。
     *
     * @param textFontData 正文字体（必填），如 Noto Sans SC
     * @param emojiFontData emoji 字体（可选），如 Noto Color Emoji
     * @throws 模块未初始化、入参无效或字体解析失败时抛出
     */
    public static registerFonts(textFontData: Uint8Array, emojiFontData?: Uint8Array): void {
        if (!(textFontData instanceof Uint8Array) || textFontData.length === 0) {
            throw new Error('SeatCanvasFont.registerFonts: textFontData must be a non-empty Uint8Array');
        }
        if (emojiFontData !== undefined &&
            (!(emojiFontData instanceof Uint8Array) || emojiFontData.length === 0)) {
            throw new Error('SeatCanvasFont.registerFonts: emojiFontData must be a non-empty Uint8Array when provided');
        }

        const module = getSeatCanvasModule();
        if (!module) {
            throw new Error('SeatCanvasFont.registerFonts must be called after SeatCanvasInit()');
        }

        const registered = module.SeatCanvasRenderer.RegisterFonts(textFontData, emojiFontData);
        if (!registered) {
            throw new Error('SeatCanvasFont.registerFonts: failed to register text font');
        }
    }
}
