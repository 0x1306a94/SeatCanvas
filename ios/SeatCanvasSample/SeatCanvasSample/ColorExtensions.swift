//
//  ColorExtensions.swift
//  SeatCanvas
//
//  Created by king on 2026/1/15.
//

import UIKit

extension UIColor {
    var rgbaHex: String {
        var red: CGFloat = 0
        var green: CGFloat = 0
        var blue: CGFloat = 0
        var alpha: CGFloat = 0

        getRed(&red, green: &green, blue: &blue, alpha: &alpha)

        let r = Int(red * 255)
        let g = Int(green * 255)
        let b = Int(blue * 255)
        let a = Int(alpha * 255)

        return String(format: "#%02X%02X%02X%02X", r, g, b, a)
    }

    var argbHex: String {
        var red: CGFloat = 0
        var green: CGFloat = 0
        var blue: CGFloat = 0
        var alpha: CGFloat = 0

        getRed(&red, green: &green, blue: &blue, alpha: &alpha)

        let r = Int(red * 255)
        let g = Int(green * 255)
        let b = Int(blue * 255)
        let a = Int(alpha * 255)

        return String(format: "#%02X%02X%02X%02X", a, r, g, b)
    }

    var rgbHex: String {
        var red: CGFloat = 0
        var green: CGFloat = 0
        var blue: CGFloat = 0
        var alpha: CGFloat = 0

        getRed(&red, green: &green, blue: &blue, alpha: &alpha)

        let r = Int(red * 255)
        let g = Int(green * 255)
        let b = Int(blue * 255)

        return String(format: "#%02X%02X%02X", r, g, b)
    }

    static func random() -> UIColor {
        UIColor(
            red: CGFloat.random(in: 0.0 ... 1.0),
            green: CGFloat.random(in: 0.0 ... 1.0),
            blue: CGFloat.random(in: 0.0 ... 1.0),
            alpha: 1.0
        )
    }

    static func color(from argbHex: String) -> UIColor? {
        var hexString = argbHex.trimmingCharacters(in: .whitespacesAndNewlines)
        if hexString.hasPrefix("#") {
            hexString.removeFirst()
        }
        guard hexString.count == 8, let hexValue = UInt32(hexString, radix: 16) else {
            return nil
        }
        let a = CGFloat((hexValue >> 24) & 0xFF) / 255.0
        let r = CGFloat((hexValue >> 16) & 0xFF) / 255.0
        let g = CGFloat((hexValue >> 8) & 0xFF) / 255.0
        let b = CGFloat(hexValue & 0xFF) / 255.0
        return UIColor(red: r, green: g, blue: b, alpha: a)
    }
}
