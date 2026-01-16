//
//  SVGSeatStyleConfig.swift
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

import Foundation

struct SVGSeatStyleConfig: SeatStyleConfig {
    let type: SeatStyleType = .svg
    let content: String
    init(content: String) {
        self.content = content
    }

    enum CodingKeys: String, CodingKey {
        case type
        case content
    }
}

extension SVGSeatStyleConfig: Encodable {
    func encode(to encoder: Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(type, forKey: .type)
        try container.encode(content, forKey: .content)
    }
}
