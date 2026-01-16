//
//  SwiftBridge.swift
//  SeatCanvas
//
//  Created by king on 2025/12/11.
//

import Foundation

// MARK: - C++ 到 Swift 的桥接函数

/// 选中座位回调，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - seatId: 座位ID
public func switf_bridge_shouldSelectSeat(coreID: UInt32, seatId: String) -> Bool {
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return false
        }

        return delegate.seatCanvasRendererShouldSelectSeat(seatId: seatId)
    }
}

/// 选中座位回调，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - seatId: 座位ID
public func switf_bridge_didSelectSeat(coreID: UInt32, seatId: String) {
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }
        delegate.seatCanvasRendererDidSelectSeat(seatId: seatId)
    }
}

/// 取消选中座位回调，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - seatId: 座位ID
public func switf_bridge_didDeselectSeat(coreID: UInt32, seatId: String) {
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }
        delegate.seatCanvasRendererDidDeselectSeat(seatId: seatId)
    }
}
