//
//  CircleSeatStyleConfig.swift
//  SeatCanvas
//
//  Created by king on 2026/1/15.
//

import Foundation

struct CircleSeatStyleConfig: SeatStyleConfig {
    let type: SeatStyleType = .circle
    let fill: PlatformColor
    let overlay: PlatformColor?
    let checkmark: PlatformColor?

    init(fill: PlatformColor, overlay: PlatformColor? = nil, checkmark: PlatformColor? = nil) {
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
