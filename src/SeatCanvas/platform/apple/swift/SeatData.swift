//
//  SeatData.swift
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

import Foundation

/// 单个座位的几何与标识数据；可变状态通过 status / selected 推送 API 更新。
@objc(KKSeatData)
@objcMembers
public class SeatData: NSObject {
    public var seatId: String
    public var position: CGPoint
    public var rotation: Float
    public var pricecode: String?

    public init(seatId: String, position: CGPoint, rotation: Float = 0, pricecode: String? = nil) {
        self.seatId = seatId
        self.position = position
        self.rotation = rotation
        self.pricecode = pricecode
    }
}
