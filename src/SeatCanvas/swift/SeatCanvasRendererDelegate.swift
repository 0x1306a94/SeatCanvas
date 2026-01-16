//
//  SeatCanvasRendererDelegate.swift
//  SeatCanvas
//
//  Created by king on 2025/12/12.
//

import Foundation

@MainActor
protocol SeatCanvasRendererDelegate: AnyObject {
    func seatCanvasRendererShouldSelectSeat(regionId: String, seatId: String) -> Bool
    func seatCanvasRendererDidSelectSeat(regionId: String, seatId: String)
    func seatCanvasRendererDidDeselectSeat(regionId: String, seatId: String)
}
