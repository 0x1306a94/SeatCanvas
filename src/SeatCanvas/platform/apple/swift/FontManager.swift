//
//  FontManager.swift
//  SeatCanvas
//
//  Created by king on 2025/11/13.
//

import Foundation

internal import SeatCanvas_Private

@objc(KKFontManager)
public final class FontManager: NSObject {
    /// 注册回退字体
    @objc
    public static func registerFallbackFonts() {
        kk.bridge.RegisterFallbackFonts()
    }
}
