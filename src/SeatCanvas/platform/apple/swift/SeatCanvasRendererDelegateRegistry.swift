//
//  SeatCanvasRendererDelegateRegistry.swift
//  SeatCanvas
//
//  Created by king on 2025/12/12.
//

import Foundation

@MainActor
final class SeatCanvasRendererDelegateRegistry {
    static let shared = SeatCanvasRendererDelegateRegistry()

    private class WeakDelegateRegistry {
        weak var delegate: SeatCanvasRendererDelegate?
        init(delegate: SeatCanvasRendererDelegate) {
            self.delegate = delegate
        }
    }

    private var registry: [UInt32: WeakDelegateRegistry] = [:]

    private init() {}

    func register(coreID: UInt32, delegate: SeatCanvasRendererDelegate) {
        let wrapper = WeakDelegateRegistry(delegate: delegate)
        registry[coreID] = wrapper
    }

    func unregister(coreID: UInt32) {
        registry.removeValue(forKey: coreID)
    }

    func cleanup() {
        registry = registry.filter {
            $0.value.delegate != nil
        }
    }

    func delegate(for coreID: UInt32) -> SeatCanvasRendererDelegate? {
        let wrapper = registry[coreID]
        return wrapper?.delegate
    }
}
