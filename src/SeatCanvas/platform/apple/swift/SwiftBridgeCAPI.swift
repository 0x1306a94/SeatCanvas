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

private func writeCString(_ string: String, to buffer: UnsafeMutablePointer<CChar>?, maxLength: Int) -> Bool {
    guard let buffer, maxLength > 0 else {
        return false
    }
    let utf8 = Array(string.utf8)
    if utf8.count + 1 > maxLength {
        return false
    }
    for (index, byte) in utf8.enumerated() {
        buffer[index] = CChar(bitPattern: byte)
    }
    buffer[utf8.count] = 0
    return true
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

@c(switf_bridge_styleIdForSeat)
func switf_bridge_styleIdForSeat(_ coreID: UInt32, _ zoneId: UnsafePointer<CChar>?, _ seatId: UnsafePointer<CChar>?, _ outStyleId: UnsafeMutablePointer<CChar>?, _ outStyleIdLen: Int) -> Bool {
    let zoneIdString = bridgeString(zoneId)
    let seatIdString = bridgeString(seatId)
    let styleId = MainActor.assumeIsolated { () -> String? in
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return nil
        }

        guard let styleId = delegate.seatCanvasRendererStyleIdForSeat(zoneId: zoneIdString, seatId: seatIdString), !styleId.isEmpty else {
            return nil
        }

        return styleId
    }

    guard let styleId else {
        return false
    }
    return writeCString(styleId, to: outStyleId, maxLength: outStyleIdLen)
}

/// 点击座位回调，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - zoneId: 区域ID
///   - seatId: 座位ID
/// - Returns: 业务层状态是否发生变化
@c(switf_bridge_didTapSeat)
func switf_bridge_didTapSeat(_ coreID: UInt32, _ zoneId: UnsafePointer<CChar>?, _ seatId: UnsafePointer<CChar>?) -> Bool {
    let zoneIdString = bridgeString(zoneId)
    let seatIdString = bridgeString(seatId)
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return false
        }

        return delegate.seatCanvasRendererDidTapSeat(zoneId: zoneIdString, seatId: seatIdString)
    }
}
