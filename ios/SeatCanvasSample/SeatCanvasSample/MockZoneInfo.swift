//
//  MockZoneInfo.swift
//  SeatCanvasSample
//
//  Created by king on 2026/1/20.
//

import CoreGraphics
import Foundation
import UIKit

class MockZoneInfo: Codable {
    let zoneId: String
    let color: UIColor?
    let rainbowColor: UIColor?
    let bounds: CGRect
    init(zoneId: String, bounds: CGRect, color: UIColor? = nil, rainbowColor: UIColor? = nil) {
        self.zoneId = zoneId
        self.bounds = bounds
        self.color = color
        self.rainbowColor = rainbowColor
    }

    enum CodingKeys: CodingKey {
        case zoneId
        case color
        case rainbowColor
        case x
        case y
        case w
        case h
    }

    required init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        zoneId = try container.decode(String.self, forKey: .zoneId)
        color = try container.decodeIfPresent(String.self, forKey: .color).flatMap { UIColor.color(from: $0) }
        rainbowColor = try container.decodeIfPresent(String.self, forKey: .rainbowColor).flatMap { UIColor.color(from: $0) }
        let x = try container.decode(CGFloat.self, forKey: .x)
        let y = try container.decode(CGFloat.self, forKey: .y)
        let w = try container.decode(CGFloat.self, forKey: .w)
        let h = try container.decode(CGFloat.self, forKey: .h)
        bounds = CGRect(x: x, y: y, width: w, height: h)
    }

    func encode(to encoder: Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(zoneId, forKey: .zoneId)
        if let color {
            try container.encode(color.rgbaHex, forKey: .color)
        }
        if let rainbowColor {
            try container.encode(rainbowColor.rgbaHex, forKey: .rainbowColor)
        }
        try container.encode(bounds.minX, forKey: .x)
        try container.encode(bounds.minY, forKey: .y)
        try container.encode(bounds.width, forKey: .w)
        try container.encode(bounds.height, forKey: .h)
    }
}
