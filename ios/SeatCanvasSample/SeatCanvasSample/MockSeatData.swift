//
//  MockSeatData.swift
//  SeatCanvasSample
//
//  Created by king on 2026/1/20.
//

import CoreGraphics
import Foundation

class MockSeatData: Codable {
    let seatId: String
    var pricecode: String
    var selected: Bool = false
    var position: CGPoint
    var rotation: Float = 0

    init(seatId: String, pricecode: String, selected: Bool, position: CGPoint) {
        self.seatId = seatId
        self.pricecode = pricecode
        self.selected = selected
        self.position = position
    }

    enum CodingKeys: CodingKey {
        case seatId
        case pricecode
        case selected
        case x
        case y
        case rotation
    }

    required init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        seatId = try container.decode(String.self, forKey: .seatId)
        pricecode = try container.decode(String.self, forKey: .pricecode)
//        self.selected = try container.decode(Bool.self, forKey: .selected)
        let x = try container.decode(CGFloat.self, forKey: .x)
        let y = try container.decode(CGFloat.self, forKey: .y)
        position = CGPoint(x: x, y: y)
        rotation = try container.decodeIfPresent(Float.self, forKey: .rotation) ?? 0.0
    }

    func encode(to encoder: Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(seatId, forKey: .seatId)
        try container.encode(pricecode, forKey: .pricecode)
//        try container.encode(selected, forKey: .selected)
        try container.encode(position.x, forKey: .x)
        try container.encode(position.y, forKey: .y)
        try container.encodeIfPresent(rotation, forKey: .rotation)
    }
}
