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
    /// 座位ID
    public var seatId: String

    /// 座位坐标(左上角)
    public var position: CGPoint

    /// 座位旋转角度
    public var rotation: Float

    /// 座位绑定价格标识
    public var pricecode: String?

    public init(seatId: String, position: CGPoint, rotation: Float = 0, pricecode: String? = nil) {
        self.seatId = seatId
        self.position = position
        self.rotation = rotation
        self.pricecode = pricecode
    }
}
