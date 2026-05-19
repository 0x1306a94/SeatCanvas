//
//  MockPriceData.swift
//  SeatCanvasSample
//
//  Created by king on 2026/5/19.
//

import CoreGraphics
import Foundation
import UIKit

class MockPriceData: Codable {
    let code: String
    let name: String
    let color: UIColor
    let zoneIds: [String]

    enum CodingKeys: CodingKey {
        case code
        case name
        case color
        case zoneIds
    }

    required init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        code = try container.decode(String.self, forKey: .code)
        name = try container.decode(String.self, forKey: .name)
        let colorHext = try container.decode(String.self, forKey: .color)
        color = UIColor.color(from: colorHext) ?? .clear
        zoneIds = try container.decode([String].self, forKey: .zoneIds)
    }

    func encode(to encoder: Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(code, forKey: .code)
        try container.encode(name, forKey: .name)
        try container.encode(color.rgbaHex, forKey: .color)
        try container.encode(zoneIds, forKey: .zoneIds)
    }
}
