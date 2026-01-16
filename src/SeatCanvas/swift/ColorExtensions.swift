//
//  ColorExtensions.swift
//  SeatCanvas
//
//  Created by king on 2026/1/15.
//

import UIKit

package extension UIColor {
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
}
