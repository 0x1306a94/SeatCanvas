//
//  SeatCanvasRendererDelegate.swift
//  SeatCanvas
//
//  Created by king on 2025/12/12.
//

import Foundation

@MainActor
protocol SeatCanvasRendererDelegate: AnyObject {
    func seatCanvasRendererShouldSelectSeat(seatId: String) -> Bool
    func seatCanvasRendererDidSelectSeat(seatId: String)
    func seatCanvasRendererDidDeselectSeat(seatId: String)
}
