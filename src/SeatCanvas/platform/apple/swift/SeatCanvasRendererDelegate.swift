//
//  SeatCanvasRendererDelegate.swift
//  SeatCanvas
//
//  Created by king on 2025/12/12.
//

import Foundation

@MainActor
protocol SeatCanvasRendererDelegate: AnyObject {
    func seatCanvasRendererShouldSelectSeat(zoneId: String, seatId: String) -> Bool
    func seatCanvasRendererDidSelectSeat(zoneId: String, seatId: String)
    func seatCanvasRendererDidDeselectSeat(zoneId: String, seatId: String)
}
