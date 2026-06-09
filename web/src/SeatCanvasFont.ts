//
//  SeatCanvasFont.ts
//  SeatCanvas
//
//  Created by king on 2026/5/27.
//

import { getSeatCanvasModule } from './SeatCanvas-module';

const defaultFontNames = [
    "Arial",
    "Courier New",
    "Georgia",
    "Times New Roman",
    "Trebuchet MS",
    "Verdana",
    "emoji",
];

export class SeatCanvasFont {
    public static registerFallbackFontNames(fontNames: string[] = []) {
        const module = getSeatCanvasModule();
        if (!module) {
            return;
        }
        const names = fontNames.length > 0 ? fontNames : defaultFontNames;
        module.SeatCanvasRenderer.SetFallbackFontNames(names);
    }
}
