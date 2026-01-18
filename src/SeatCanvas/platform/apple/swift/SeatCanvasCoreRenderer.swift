//
//  SeatCanvasCoreRenderer.swift
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

import Foundation

internal import SeatCanvas_Private

@MainActor
final class SeatCanvasCoreRenderer {
    private(set) nonisolated(unsafe) var cppObject: UnsafeMutablePointer<kk.bridge.CPPObject>?
    private(set) nonisolated(unsafe) var coreID: UInt32 = 0
    init(eaglLayer: CAEAGLLayer?) {
        let cppObject = kk.bridge.CreateSeatCanvasCoreRenderer(eaglLayer)
        coreID = kk.bridge.SeatCanvasCoreRendererGetCoreID(cppObject)
        self.cppObject = cppObject
    }

    var backgroundColor: UIColor {
        set {
            guard let cppObject = cppObject else {
                return
            }
            kk.bridge.SeatCanvasCoreRendererSetBackgroundColor(cppObject, newValue)
        }
        get {
            guard let cppObject = cppObject else {
                return .clear
            }

            return kk.bridge.SeatCanvasCoreRendererGetBackgroundColor(cppObject)
        }
    }

    deinit {
        guard var cppObject = self.cppObject else {
            return
        }
        kk.bridge.ReleaseCPPObject(&cppObject)
    }
}

extension SeatCanvasCoreRenderer {
    func replacePlatformView(eaglLayer: CAEAGLLayer?) -> Bool {
        guard let cppObject = cppObject else {
            return false
        }
        return kk.bridge.SeatCanvasCoreRendererReplacePlatformView(cppObject, eaglLayer)
    }

    func loadBaseMap(_ data: Data?, format: BaseMapFormat) {
        guard let cppObject = cppObject else {
            return
        }

        guard let data else {
            kk.bridge.SeatCanvasCoreRendererLoadBaseMap(cppObject, nil)
            return
        }

        let cppFormat = format.cppFormat
        guard cppFormat != .Unknown else {
            return
        }

        DispatchQueue.global(qos: .userInteractive).async {
            var loadResult = data.withUnsafeBytes { buffer in
                let result = kk.bridge.SeatCanvasCoreRendererParseBaseMap(buffer.baseAddress, buffer.count, cppFormat, nil)
                return result
            }

            DispatchQueue.main.async { [weak self] in
                guard let cppObject = self?.cppObject else {
                    return
                }
                kk.bridge.SeatCanvasCoreRendererLoadBaseMap(cppObject, &loadResult)
            }
        }
    }

    func applySeatStyleJSONConfig(_ data: Data?) {
        guard let cppObject = cppObject else {
            return
        }

        guard let data, !data.isEmpty else {
            kk.bridge.SeatCanvasCoreRendererSetSeatStyleJSONConfig(cppObject, nil, 0)
            return
        }

        #if DEBUG
            print("coreID: \(coreID) SeatStyleJSON: \n\(String(data: data, encoding: .utf8)!)")
        #endif
        data.withUnsafeBytes { buffer in
            kk.bridge.SeatCanvasCoreRendererSetSeatStyleJSONConfig(cppObject, buffer.baseAddress, buffer.count)
        }
    }

    func handTap(location: CGPoint) {
        guard let cppObject = cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererHandTap(cppObject, location)
    }

    func handPan(state: kk.gesture.GestureState, translation: CGPoint, timestamp: Double) {
        guard let cppObject = cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererHandPan(cppObject, state, translation, timestamp)
    }

    func handPinch(state: kk.gesture.GestureState, scale: CGFloat, center: CGPoint) {
        guard let cppObject = cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererHandPinch(cppObject, state, scale, center)
    }

    func updateSize() -> Bool {
        guard let cppObject = cppObject else {
            return false
        }
        return kk.bridge.SeatCanvasCoreRendererUpdateSize(cppObject)
    }

    func startDrawLoop() {
        guard let cppObject = cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererStart(cppObject)
    }

    func stopDrawLoop() {
        guard let cppObject = cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererStop(cppObject)
    }

    func invalidateContent() {
        guard let cppObject = cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererInvalidateContent(cppObject)
    }
}
