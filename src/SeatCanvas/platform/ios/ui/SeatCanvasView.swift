//
//  SeatCanvasView.swift
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

import Foundation
import UIKit

internal import SeatCanvas_Private

@MainActor
@objc(KKSeatCanvasView)
public class SeatCanvasView: UIView {
    var renderView: SeatCanvasRenderView!
    var isSmallVenue = false
    var minimapImage: UIImage?

    nonisolated(unsafe) var renderer: SeatCanvasCoreRenderer?

    @objc
    public weak var delegate: SeatCanvasViewDelegate?

    var tapGestureRecognizer: UITapGestureRecognizer!
    var panGestureRecognizer: UIPanGestureRecognizer!
    var pinchGestureRecognizer: UIPinchGestureRecognizer!

    public var canvasColor: UIColor? {
        set {
            renderer?.backgroundColor = newValue
        }
        get {
            renderer?.backgroundColor ?? .clear
        }
    }

    override public init(frame: CGRect) {
        super.init(frame: frame)
        commonInit()
    }

    public required init?(coder: NSCoder) {
        super.init(coder: coder)
        commonInit()
    }

    override public func didMoveToWindow() {
        super.didMoveToWindow()
        if let _ = window {
            renderer?.invalidateContent()
            renderer?.startDrawLoop()
        } else {
            renderer?.stopDrawLoop()
        }
    }

    /// 加载底图（指定格式）
    /// - Parameters:
    ///   - data: 底图数据
    ///   - format: 格式
    @objc
    public func loadBaseMap(_ data: Data?, format: BaseMapFormat) {
        renderer?.loadBaseMap(data, format: format)
    }

    /// 应用样式配置（使用 SeatStyleConfigBuilder 构建）
    /// - Parameter data: json 样式数据
    @objc
    public func applySeatStyleJSONConfig(_ data: Data?) {
        renderer?.applySeatStyleJSONConfig(data)
    }

    /// 更新座位区域信息
    /// - Parameter zones: 区域集合
    @objc
    public func updateSeatZoneDatas(zones: [SeatZoneData]) {
        renderer?.updateSeatZoneDatas(zones: zones)
        renderer?.invalidateContent()
    }

    /// 更新区域座位信息
    /// - Parameters:
    ///   - zoneId: 区域ID
    ///   - seats: 座位集合
    @objc
    public func updateSeatDatas(zoneId: String, seats: [SeatData]) {
        renderer?.updateSeatDatas(zoneId: zoneId, seats: seats)
        renderer?.invalidateContent()
    }

    /// 高亮指定区域：高亮区域 additionalAlpha=1.0，其余为 nonHighlightedAlpha
    /// - Parameters:
    ///   - zoneIds: 高亮区域的 ID 集合
    ///   - nonHighlightedAlpha: 非高亮区域的 additionalAlpha
    @objc
    public func setHighlightedZoneIds(_ zoneIds: [String], nonHighlightedAlpha: Float) {
        renderer?.setHighlightedZoneIds(zoneIds, nonHighlightedAlpha: nonHighlightedAlpha)
    }

    /// 清除高亮，将所有区域 additionalAlpha 重置为 1.0
    @objc
    public func clearHighlightedZones() {
        renderer?.clearHighlightedZones()
    }

    /// 座位大小
    @objc
    public var seatSize: CGFloat {
        get {
            renderer?.seatSize ?? 36.0
        }
        set {
            renderer?.seatSize = newValue
        }
    }

    /// 座位渲染阈值（控制从彩虹图切换到绘制座位的缩放级别）
    /// 默认等同于内部计算得到的 ZoomLevelConfig.venue
    @objc
    public var seatRenderZoomThreshold: CGFloat {
        get {
            renderer?.seatRenderZoomThreshold ?? 0.0
        }
        set {
            renderer?.seatRenderZoomThreshold = newValue
        }
    }

    /// 获取缩放级别
    @objc
    public func zoomLevel() -> ZoomLevel {
        guard let renderer else {
            return ZoomLevel.defalut
        }
        return renderer.zoomLevel
    }

    /// 最小缩放级别
    @objc
    public var minimumZoomScale: CGFloat {
        renderer?.minimumZoomScale ?? 1.0
    }

    /// 最大缩放级别
    @objc
    public var maximumZoomScale: CGFloat {
        renderer?.maximumZoomScale ?? 1.0
    }

    deinit {
        NotificationCenter.default.removeObserver(self, name: UIApplication.didEnterBackgroundNotification, object: nil)
        NotificationCenter.default.removeObserver(self, name: UIApplication.willEnterForegroundNotification, object: nil)

        let coreID = self.renderer?.coreID ?? 0
        if coreID != 0 {
            Task { @MainActor in
                SeatCanvasRendererDelegateRegistry.shared.unregister(coreID: coreID)
            }
        }

        #if DEBUG
            print("\(type(of: self)) deinit, coreID: \(coreID)")
        #endif
    }
}

extension SeatCanvasView {
    func commonInit() {
        setupSystemProperties()

        setupViews()
        setupGestureRecognizer()
        setupNotification()

        setupRenderer()

        #if DEBUG
            print("[SeatCanvasView] 初始化完成, coreID: \(renderer?.coreID ?? 0)")
        #endif
    }

    func setupSystemProperties() {
        let density = UIScreen.main.scale
        let fontScale = UIScreen.main.scale
        kk.bridge.SeatCanvasInitSystemProperties(density, fontScale)
    }

    func setupViews() {
        renderView = SeatCanvasRenderView(frame: bounds)
        renderView.translatesAutoresizingMaskIntoConstraints = false
        renderView.contentScaleFactor = UIScreen.main.scale
        renderView.didUpdateSize = { [weak self] _ in
            self?.handleUpdateSize()
        }

        addSubview(renderView)

        NSLayoutConstraint.activate([
            renderView.leadingAnchor.constraint(equalTo: leadingAnchor),
            renderView.topAnchor.constraint(equalTo: topAnchor),
            renderView.trailingAnchor.constraint(equalTo: trailingAnchor),
            renderView.bottomAnchor.constraint(equalTo: bottomAnchor),
        ])
    }

    func setupGestureRecognizer() {
        tapGestureRecognizer = UITapGestureRecognizer(target: self, action: #selector(handleTapGestureRecognizer(gesture:)))

        panGestureRecognizer = UIPanGestureRecognizer(target: self, action: #selector(handlePanGestureRecognizer(gesture:)))
        pinchGestureRecognizer = UIPinchGestureRecognizer(target: self, action: #selector(handlePinchGestureRecognizer(gesture:)))

        renderView.addGestureRecognizer(panGestureRecognizer)
        renderView.addGestureRecognizer(pinchGestureRecognizer)

        tapGestureRecognizer.require(toFail: panGestureRecognizer)

        renderView.addGestureRecognizer(tapGestureRecognizer)
    }

    func setupRenderer() {
        renderer = SeatCanvasCoreRenderer(eaglLayer: renderView.layer as? CAEAGLLayer)
        if let coreID = renderer?.coreID {
            SeatCanvasRendererDelegateRegistry.shared.register(coreID: coreID, delegate: self)
        }
        renderer?.backgroundColor = .white
    }

    func setupNotification() {
        NotificationCenter.default.addObserver(self, selector: #selector(appDidEnterBackground(notification:)), name: UIApplication.didEnterBackgroundNotification, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(appWillEnterForeground(notification:)), name: UIApplication.willEnterForegroundNotification, object: nil)
    }

//    func convertToCanvas(_ point: CGPoint) -> CGPoint {
//        let currentZoom = kk.bridge.SeatCanvasCoreRendererZoomScale(rendererCore)
//        let contentOffset = kk.bridge.SeatCanvasCoreRendereContentOffset(rendererCore)
//        let density = kk.bridge.SeatCanvasCoreRendererGetDensity(rendererCore)
//
//        let px = point.x * density
//        let py = point.y * density
//        let x = (px - contentOffset.x) / currentZoom
//        let y = (py - contentOffset.y) / currentZoom
//
//        return CGPointMake(x, y)
//    }

    @objc
    func handleTapGestureRecognizer(gesture: UITapGestureRecognizer) {
        guard gesture.state == .ended else {
            return
        }

        var location = gesture.location(in: renderView)
        let contentScaleFactor = renderView.contentScaleFactor
        location.x *= contentScaleFactor
        location.y *= contentScaleFactor
        renderer?.handTap(location: location)
    }

    @objc
    func handlePanGestureRecognizer(gesture: UIPanGestureRecognizer) {
        guard gesture.state != .possible, gesture.state != .failed else {
            return
        }

        var translation = gesture.translation(in: renderView)
        let contentScaleFactor = -renderView.contentScaleFactor
        translation.x *= contentScaleFactor
        translation.y *= contentScaleFactor

        let timestampMs = CACurrentMediaTime() * 1000.0
        let state = gesture.state
        switch state {
        case .began:
            renderer?.handPan(state: kk.gesture.GestureState.BEGAN, translation: translation, timestamp: timestampMs)
        case .changed:
            renderer?.handPan(state: kk.gesture.GestureState.CHANGED, translation: translation, timestamp: timestampMs)
        case .ended, .cancelled:
            renderer?.handPan(state: (state == .ended) ? kk.gesture.GestureState.ENDED : kk.gesture.GestureState.CANCELLED, translation: translation, timestamp: timestampMs)
            gesture.setTranslation(.zero, in: renderView)
        default:
            break
        }
    }

    @objc
    func handlePinchGestureRecognizer(gesture: UIPinchGestureRecognizer) {
        guard gesture.state != .possible, gesture.state != .failed else {
            return
        }

        var center = gesture.location(in: renderView)
        let contentScaleFactor = renderView.contentScaleFactor
        center.x *= contentScaleFactor
        center.y *= contentScaleFactor

        let state = gesture.state
        switch state {
        case .began:
            gesture.scale = 1.0
            renderer?.handPinch(state: kk.gesture.GestureState.BEGAN, scale: 1.0, center: center)
        case .changed:
            let scale = gesture.scale
            renderer?.handPinch(state: kk.gesture.GestureState.CHANGED, scale: scale, center: center)
        case .ended, .cancelled:
            let scale = gesture.scale

            renderer?.handPinch(state: (state == .ended) ? kk.gesture.GestureState.ENDED : kk.gesture.GestureState.CANCELLED, scale: scale, center: center)
        default:
            break
        }
    }

    @objc
    func appDidEnterBackground(notification _: Notification) {
        renderer?.stopDrawLoop()
    }

    @objc
    func appWillEnterForeground(notification _: Notification) {
        renderer?.startDrawLoop()
    }

    func handleUpdateSize() {
        _ = renderer?.updateSize()
    }

//    func calculateVisibleContentRect() {
//        let baseMapScale = kk.bridge.SeatCanvasCoreRendererBaseMapScale(rendererCore)
//
//        // C++ 计算
//        var cppVisibleContentRect = kk.bridge.SeatCanvasCoreRendererGetVisibleContentRect(rendererCore)
//        let inverseScale = 1.0 / baseMapScale
//        let transform = CGAffineTransform(scaleX: inverseScale, y: inverseScale)
//        cppVisibleContentRect = cppVisibleContentRect.applying(transform)
    ////        print("visibleContentRect: \(cppVisibleContentRect)")
//    }
}

// MARK: - C++ 回调处理

extension SeatCanvasView: SeatCanvasRendererDelegate {
    func seatCanvasRendererDidTapZone(zoneId: String) {
        delegate?.seatCanvasView(self, didTapZone: zoneId)
    }

    func seatCanvasRendererShouldSelectSeat(zoneId: String, seatId: String) -> Bool {
        delegate?.seatCanvasView(self, shouldSelectSeat: zoneId, seatId: seatId) ?? false
    }

    func seatCanvasRendererDidSelectSeat(zoneId: String, seatId: String) {
        delegate?.seatCanvasView(self, didSelectSeat: zoneId, seatId: seatId)
    }

    func seatCanvasRendererDidDeselectSeat(zoneId: String, seatId: String) {
        delegate?.seatCanvasView(self, didDeselectSeat: zoneId, seatId: seatId)
    }
}
