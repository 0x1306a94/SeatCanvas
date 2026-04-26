//
//  CircleSeatStyleConfig.swift
//  SeatCanvas
//
//  Created by king on 2026/1/15.
//

import UIKit

struct CircleSeatStyleConfig: SeatStyleConfig {
    let type: SeatStyleType = .circle
    let fill: UIColor
    let overlay: UIColor?
    let checkmark: UIColor?

    init(fill: UIColor, overlay: UIColor? = nil, checkmark: UIColor? = nil) {
        self.fill = fill
        self.overlay = overlay
        self.checkmark = checkmark
    }

    enum CodingKeys: String, CodingKey {
        case type
        case fill
        case overlay
        case checkmark
    }
}

extension CircleSeatStyleConfig: Encodable {
    func encode(to encoder: Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(type, forKey: .type)
        try container.encode(fill.argbHex, forKey: .fill)
        try container.encodeIfPresent(overlay?.argbHex, forKey: .overlay)
        try container.encodeIfPresent(checkmark?.argbHex, forKey: .checkmark)
    }
}
