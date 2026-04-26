//
//  SwiftBridgeCAPI.swift
//  SeatCanvas
//
//  Created by king on 2025/12/11.
//

import Foundation

// MARK: - C++ 到 Swift 的桥接函数

private func bridgeString(_ value: UnsafePointer<CChar>?) -> String {
    guard let value else {
        return ""
    }
    return String(cString: value)
}

/// 点击区域回调，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - zoneId: 区域ID
@c(switf_bridge_didTapZone)
func switf_bridge_didTapZone(_ coreID: UInt32, _ zoneId: UnsafePointer<CChar>?) {
    let zoneIdString = bridgeString(zoneId)
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }

        delegate.seatCanvasRendererDidTapZone(zoneId: zoneIdString)
    }
}

/// 选中座位回调，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - zoneId: 区域ID
///   - seatId: 座位ID
@c(switf_bridge_shouldSelectSeat)
func switf_bridge_shouldSelectSeat(_ coreID: UInt32, _ zoneId: UnsafePointer<CChar>?, _ seatId: UnsafePointer<CChar>?) -> Bool {
    let zoneIdString = bridgeString(zoneId)
    let seatIdString = bridgeString(seatId)
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return false
        }

        return delegate.seatCanvasRendererShouldSelectSeat(zoneId: zoneIdString, seatId: seatIdString)
    }
}

/// 选中座位回调，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - zoneId: 区域ID
///   - seatId: 座位ID
@c(switf_bridge_didSelectSeat)
func switf_bridge_didSelectSeat(_ coreID: UInt32, _ zoneId: UnsafePointer<CChar>?, _ seatId: UnsafePointer<CChar>?) {
    let zoneIdString = bridgeString(zoneId)
    let seatIdString = bridgeString(seatId)
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }

        delegate.seatCanvasRendererDidSelectSeat(zoneId: zoneIdString, seatId: seatIdString)
    }
}

/// 是否可以选中座位，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - zoneId: 区域ID
///   - seatId: 座位ID
@c(switf_bridge_didDeselectSeat)
func switf_bridge_didDeselectSeat(_ coreID: UInt32, _ zoneId: UnsafePointer<CChar>?, _ seatId: UnsafePointer<CChar>?) {
    let zoneIdString = bridgeString(zoneId)
    let seatIdString = bridgeString(seatId)
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }

        delegate.seatCanvasRendererDidDeselectSeat(zoneId: zoneIdString, seatId: seatIdString)
    }
}
