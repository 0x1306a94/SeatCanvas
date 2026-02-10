//
//  SeatCanvasCoreRenderer.swift
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

import Foundation
internal import CxxStdlib
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

    var backgroundColor: UIColor? {
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

    /// 座位渲染阈值（控制从彩虹图切换到绘制座位的缩放级别）
    var seatRenderZoomThreshold: CGFloat {
        get {
            guard let cppObject = cppObject else {
                return 0.0
            }
            return kk.bridge.SeatCanvasCoreRendererGetSeatRenderZoomThreshold(cppObject)
        }
        set {
            guard let cppObject = cppObject else {
                return
            }
            kk.bridge.SeatCanvasCoreRendererSetSeatRenderZoomThreshold(cppObject, newValue)
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

    func setHighlightedZoneIds(_ zoneIds: [String], nonHighlightedAlpha: Float) {
        guard let cppObject = cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererSetHighlightedZoneIds(cppObject, zoneIds, nonHighlightedAlpha)
    }

    func clearHighlightedZones() {
        guard let cppObject = cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererClearHighlightedZones(cppObject)
    }

    var seatSize: CGFloat {
        get {
            guard let cppObject = cppObject else {
                return 36.0
            }
            return kk.bridge.SeatCanvasCoreRendererGetSeatSize(cppObject)
        }
        set {
            guard let cppObject = cppObject else {
                return
            }
            kk.bridge.SeatCanvasCoreRendererSetSeatSize(cppObject, newValue)
        }
    }

    func updateSeatZoneDatas(zones: [SeatZoneData]) {
        guard let cppObject = cppObject else {
            return
        }

        var builder = kk.bridge.CreateSeatZoneDataBuilder()
        for zone in zones {
            kk.bridge.SeatZoneDataBuilderPut(builder, std.string(zone.zoneId), zone.color, zone.rainbowColor)
        }

        kk.bridge.SeatCanvasCoreRendererSetSeatZoneDatas(cppObject, builder)
        kk.bridge.ReleaseCPPObject(&builder)
    }

    func updateSeatDatas(zoneId: String, seats: [SeatData]) {
        guard let cppObject = cppObject else {
            return
        }

        var builder = kk.bridge.CreateSeatDataBuilder()
        for seat in seats {
            kk.bridge.SeatDataBuilderPut(builder, std.string(seat.seatId), seat.status, seat.selected, seat.position)
        }

        kk.bridge.SeatCanvasCoreRendererSetSeatDatas(cppObject, std.string(zoneId), builder)
        kk.bridge.ReleaseCPPObject(&builder)
    }
}
