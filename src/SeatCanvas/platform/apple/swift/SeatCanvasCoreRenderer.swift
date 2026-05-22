//
//  SeatCanvasCoreRenderer.swift
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

import Foundation
import MetalKit
internal import SeatCanvas_Private

@MainActor
final class SeatCanvasCoreRenderer {
    private(set) nonisolated(unsafe) var cppObject: UnsafeMutablePointer<kk.bridge.CPPObject>?
    private(set) nonisolated(unsafe) var coreID: UInt32 = 0
    init(metalView: MTKView?) {
        let cppObject = kk.bridge.CreateSeatCanvasCoreRenderer(metalView)
        coreID = kk.bridge.SeatCanvasCoreRendererGetCoreID(cppObject)
        self.cppObject = cppObject
    }

    /// Temporary warmup entry for shader compiler initialization.
    /// TODO: Remove this API after warmup is integrated into renderer startup flow.
    nonisolated static func prewarmShaderCompiler() {
        kk.bridge.SeatCanvasCoreRendererPrewarmShaderCompiler()
    }

    var backgroundColor: UIColor? {
        set {
            guard let cppObject else {
                return
            }
            kk.bridge.SeatCanvasCoreRendererSetBackgroundColor(cppObject, newValue)
        }
        get {
            guard let cppObject else {
                return .clear
            }

            return kk.bridge.SeatCanvasCoreRendererGetBackgroundColor(cppObject)
        }
    }

    /// 座位渲染阈值（控制从彩虹图切换到绘制座位的缩放级别）
    var seatRenderZoomThreshold: CGFloat {
        get {
            guard let cppObject else {
                return 0.0
            }
            return kk.bridge.SeatCanvasCoreRendererGetSeatRenderZoomThreshold(cppObject)
        }
        set {
            guard let cppObject else {
                return
            }
            kk.bridge.SeatCanvasCoreRendererSetSeatRenderZoomThreshold(cppObject, newValue)
        }
    }

    var debugHUDEnabled: Bool {
        get {
            guard let cppObject else {
                return false
            }
            return kk.bridge.SeatCanvasCoreRendererIsDebugHUDEnabled(cppObject)
        }
        set {
            guard let cppObject else {
                return
            }
            kk.bridge.SeatCanvasCoreRendererSetDebugHUDEnabled(cppObject, newValue)
        }
    }

    var minimumZoomScale: CGFloat {
        guard let cppObject else {
            return 1.0
        }
        return kk.bridge.SeatCanvasCoreRendererMinimumZoomScale(cppObject)
    }

    var zoomScale: CGFloat {
        guard let cppObject else {
            return 1.0
        }
        return kk.bridge.SeatCanvasCoreRendererZoomScale(cppObject)
    }

    var maximumZoomScale: CGFloat {
        guard let cppObject else {
            return 1.0
        }
        return kk.bridge.SeatCanvasCoreRendererMaximumZoomScale(cppObject)
    }

    var zoomLevel: ZoomLevel {
        guard let cppObject else {
            return ZoomLevel.defalut
        }

        let result = kk.bridge.SeatCanvasCoreRendererGetZoomLevel(cppObject)
        return ZoomLevel(seat: result.seat, row: result.row, zone: result.zone, venue: result.venue)
    }

    deinit {
        guard var cppObject = self.cppObject else {
            return
        }
        kk.bridge.ReleaseCPPObject(&cppObject)
    }
}

extension SeatCanvasCoreRenderer {
    func replacePlatformView(metalView: MTKView?) -> Bool {
        guard let cppObject else {
            return false
        }
        return kk.bridge.SeatCanvasCoreRendererReplacePlatformView(cppObject, metalView)
    }

    func loadBaseMap(_ data: Data?, format: BaseMapFormat, parseConfigJSON: Data?) {
        guard let cppObject else {
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
            var loadResult: UnsafeMutableRawPointer?
            if let parseConfigJSON {
                loadResult = parseConfigJSON.withUnsafeBytes { parseConfigBuffer in
                    data.withUnsafeBytes { buffer in
                        kk.bridge.SeatCanvasCoreRendererParseBaseMap(
                            buffer.baseAddress,
                            buffer.count,
                            cppFormat,
                            parseConfigBuffer.baseAddress,
                            parseConfigBuffer.count,
                            nil
                        )
                    }
                }
            } else {
                loadResult = data.withUnsafeBytes { buffer in
                    kk.bridge.SeatCanvasCoreRendererParseBaseMap(
                        buffer.baseAddress,
                        buffer.count,
                        cppFormat,
                        nil,
                        0,
                        nil
                    )
                }
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
        guard let cppObject else {
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
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererHandTap(cppObject, location)
    }

    func handPan(state: kk.gesture.GestureState, translation: CGPoint, timestamp: Double) {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererHandPan(cppObject, state, translation, timestamp)
    }

    func handPinch(state: kk.gesture.GestureState, scale: CGFloat, center: CGPoint) {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererHandPinch(cppObject, state, scale, center)
    }

    func updateSize() -> Bool {
        guard let cppObject else {
            return false
        }
        return kk.bridge.SeatCanvasCoreRendererUpdateSize(cppObject)
    }

    func startDrawLoop() {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererStart(cppObject)
    }

    func stopDrawLoop() {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererStop(cppObject)
    }

    func invalidateContent() {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererInvalidateContent(cppObject)
    }

    var seatSize: CGFloat {
        get {
            guard let cppObject else {
                return 36.0
            }
            return kk.bridge.SeatCanvasCoreRendererGetSeatSize(cppObject)
        }
        set {
            guard let cppObject else {
                return
            }
            kk.bridge.SeatCanvasCoreRendererSetSeatSize(cppObject, newValue)
        }
    }

    func updateSeatZoneAlternateColors(colors: [String: UIColor]?) {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererSetSeatZoneAlternateColors(cppObject, colors)
    }

    func updateMiniMapZoneAlternateColors(colors: [String: UIColor]?) {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererSetMiniMapZoneAlternateColors(cppObject, colors)
    }

    func updateSeatDatas(zoneId: String, seats: [SeatData]) {
        guard let cppObject else {
            return
        }

        var builder = kk.bridge.CreateSeatDataBuilder()
        for seat in seats {
            let pricecodeIndex = kk.bridge.SeatCanvasCoreRendererPricecodeIndexForCode(cppObject, seat.pricecode ?? "")
            kk.bridge.SeatDataBuilderPut(builder, seat.seatId, seat.position, seat.rotation, pricecodeIndex)
        }

        kk.bridge.SeatCanvasCoreRendererSetSeatDatas(cppObject, zoneId, builder)
        kk.bridge.ReleaseCPPObject(&builder)
    }

    func registerPricecodes(_ pricecodes: [String]) {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererRegisterPricecodes(cppObject, pricecodes)
    }

    func updateSeatStatuses(seatIds: [String], statuses: [UInt32]) {
        guard let cppObject else {
            return
        }
        let statusNumbers = statuses.map { NSNumber(value: $0) }
        kk.bridge.SeatCanvasCoreRendererUpdateSeatStatuses(cppObject, seatIds, statusNumbers)
    }

    func updateSeatStatusesForZone(zoneId: String, statuses: [UInt32]) {
        guard let cppObject else {
            return
        }
        let statusNumbers = statuses.map { NSNumber(value: $0) }
        kk.bridge.SeatCanvasCoreRendererUpdateSeatStatusesForZone(cppObject, zoneId, statusNumbers)
    }

    func setSelectedSeatIds(_ seatIds: [String]) {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererSetSelectedSeatIds(cppObject, seatIds)
    }

    func updateSelectedSeatIds(added: [String], removed: [String]) {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererUpdateSelectedSeatIds(cppObject, added, removed)
    }

    func clearSeatData() {
        guard let cppObject else {
            return
        }
        kk.bridge.SeatCanvasCoreRendererClearSeatData(cppObject)
    }

    func visibleOriginalRect() -> CGRect {
        guard let cppObject else {
            return .zero
        }
        return kk.bridge.SeatCanvasCoreRendererGetVisibleContentRect(cppObject)
    }

    func zoneIds(inOriginalRect rect: CGRect) -> [String] {
        guard let cppObject else {
            return []
        }
        let zoneIds = kk.bridge.SeatCanvasCoreRendererGetZoneIdsInOriginalRect(cppObject, rect)
        return zoneIds
    }
}
