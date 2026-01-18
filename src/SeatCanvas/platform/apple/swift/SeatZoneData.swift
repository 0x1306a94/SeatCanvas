//
//  SeatZoneData.swift
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

import UIKit

@objc(KKSeatZoneData)
@objcMembers
public class SeatZoneData: NSObject {
    public var zoneId: String
    public var color: UIColor?
    public var pirceColor: UIColor?
    public init(zoneId: String, color: UIColor? = nil, pirceColor: UIColor? = nil) {
        self.zoneId = zoneId
        self.color = color
        self.pirceColor = pirceColor
    }
}
