//
//  SeatData.swift
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

import Foundation

@objc(KKSeatData)
@objcMembers
public class SeatData: NSObject {
    public var seatId: String
    public var position: CGPoint
    public var rotation: Float

    public init(seatId: String, position: CGPoint, rotation: Float = 0) {
        self.seatId = seatId
        self.position = position
        self.rotation = rotation
    }
}
