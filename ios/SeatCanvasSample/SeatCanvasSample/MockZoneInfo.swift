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
    let alternateColor: UIColor?
    let bounds: CGRect
    init(zoneId: String, bounds: CGRect, alternateColor: UIColor? = nil) {
        self.zoneId = zoneId
        self.bounds = bounds
        self.alternateColor = alternateColor
    }

    enum CodingKeys: CodingKey {
        case zoneId
        case alternateColor
        case x
        case y
        case w
        case h
    }

    required init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        zoneId = try container.decode(String.self, forKey: .zoneId)
        alternateColor = try container.decodeIfPresent(String.self, forKey: .alternateColor).flatMap { UIColor.color(from: $0) }
        let x = try container.decode(CGFloat.self, forKey: .x)
        let y = try container.decode(CGFloat.self, forKey: .y)
        let w = try container.decode(CGFloat.self, forKey: .w)
        let h = try container.decode(CGFloat.self, forKey: .h)
        bounds = CGRect(x: x, y: y, width: w, height: h)
    }

    func encode(to encoder: Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(zoneId, forKey: .zoneId)
        if let alternateColor {
            try container.encode(alternateColor.rgbaHex, forKey: .alternateColor)
        }
        try container.encode(bounds.minX, forKey: .x)
        try container.encode(bounds.minY, forKey: .y)
        try container.encode(bounds.width, forKey: .w)
        try container.encode(bounds.height, forKey: .h)
    }
}
