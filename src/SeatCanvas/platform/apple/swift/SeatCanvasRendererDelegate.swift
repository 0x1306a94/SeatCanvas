//
//  SeatCanvasRendererDelegate.swift
//  SeatCanvas
//
//  Created by king on 2025/12/12.
//

import Foundation

@MainActor
protocol SeatCanvasRendererDelegate: AnyObject {
    func seatCanvasRendererDidTapZone(zoneId: String)
    func seatCanvasRendererStyleIdForSeat(zoneId: String, seatId: String) -> String?
    func seatCanvasRendererDidTapSeat(zoneId: String, seatId: String) -> Bool
}
