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
///   - regionId: 区域ID
///   - seatId: 座位ID
public func switf_bridge_shouldSelectSeat(coreID: UInt32, regionId: String, seatId: String) -> Bool {
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
#if DEBUG
            print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
#endif
            return false
        }

        return delegate.seatCanvasRendererShouldSelectSeat(regionId: regionId, seatId: seatId)
    }
}

/// 选中座位回调，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - regionId: 区域ID
///   - seatId: 座位ID
public func switf_bridge_didSelectSeat(coreID: UInt32, regionId: String, seatId: String) {
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
#if DEBUG
            print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
#endif
            return
        }

        delegate.seatCanvasRendererDidSelectSeat(regionId: regionId, seatId: seatId)
    }
}

/// 是否可以选中座位，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - regionId: 区域ID
///   - seatId: 座位ID
public func switf_bridge_didDeselectSeat(coreID: UInt32, regionId: String, seatId: String) {
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
#if DEBUG
            print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
#endif
            return
        }

        delegate.seatCanvasRendererDidDeselectSeat(regionId: regionId, seatId: seatId)
    }
}
