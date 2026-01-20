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
    var status: UInt32
    var selected: Bool = false
    var position: CGPoint

    init(seatId: String, status: UInt32, selected: Bool, position: CGPoint) {
        self.seatId = seatId
        self.status = status
        self.selected = selected
        self.position = position
    }

    enum CodingKeys: CodingKey {
        case seatId
        case status
        case selected
        case x
        case y
    }

    required init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        seatId = try container.decode(String.self, forKey: .seatId)
        status = try container.decode(UInt32.self, forKey: .status)
//        self.selected = try container.decode(Bool.self, forKey: .selected)
        let x = try container.decode(CGFloat.self, forKey: .x)
        let y = try container.decode(CGFloat.self, forKey: .y)
        position = CGPoint(x: x, y: y)
    }

    func encode(to encoder: Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try container.encode(seatId, forKey: .seatId)
        try container.encode(status, forKey: .status)
//        try container.encode(selected, forKey: .selected)
        try container.encode(position.x, forKey: .x)
        try container.encode(position.y, forKey: .y)
    }
}
