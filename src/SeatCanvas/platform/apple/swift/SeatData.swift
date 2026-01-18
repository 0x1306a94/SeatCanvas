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
    public var status: UInt32
    public var selected: Bool
    public var position: CGPoint

    public init(seatId: String, status: UInt32, selected: Bool, position: CGPoint) {
        self.seatId = seatId
        self.status = status
        self.selected = selected
        self.position = position
    }
}
