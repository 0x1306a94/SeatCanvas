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
    var zoomLevel: kk.ZoomLevel = .init(zoomScale9: 1.0, zoomScale18: 1.0, zoomScale30: 1.0, zoomScale50: 1.0)
    var isSmallVenue = false
    var minimapImage: UIImage?

    nonisolated(unsafe) var rendererCore: UnsafeMutablePointer<kk.CPPObject>!
    nonisolated(unsafe) var coreID: UInt32 = 0

    /// 座位选中事件代理
    @objc
    public weak var delegate: SeatCanvasViewDelegate?

    var tapGestureRecognizer: UITapGestureRecognizer!
    var panGestureRecognizer: UIPanGestureRecognizer!
    var pinchGestureRecognizer: UIPinchGestureRecognizer!

    override public var backgroundColor: UIColor? {
        didSet {
            guard let rendererCore else {
                return
            }
            kk.SeatCanvasCoreRendererSetBackgroundColor(rendererCore, backgroundColor)
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
            rendererInvalidateContent()
            startDrawLoop()
        } else {
            stopDrawLoop()
        }
    }

    /// 加载底图（指定格式）
    /// - Parameters:
    ///   - data: 底图数据
    ///   - format: 格式
    @objc
    public func loadBaseMap(_ data: Data?, format: BaseMapFormat) {
        guard let data else {
            kk.SeatCanvasCoreRendererLoadBaseMap(rendererCore, nil)
            return
        }

        let cppFormat = format.cppFormat
        guard cppFormat != .Unknown else {
            return
        }

        DispatchQueue.global(qos: .userInteractive).async {
            var minimapImage: UIImage? = nil
            var loadResult = data.withUnsafeBytes { buffer in
                // 使用统一接口
                let result = kk.SeatCanvasLoadBaseMap(buffer.baseAddress, buffer.count, cppFormat, &minimapImage)
                return result
            }

            DispatchQueue.main.async { [weak self] in
                guard let self else { return }
                self.minimapImage = minimapImage
                kk.SeatCanvasCoreRendererLoadBaseMap(self.rendererCore, &loadResult)
            }
        }
    }

    /// 应用样式配置（使用 SeatStyleConfigBuilder 构建）
    /// - Parameter data: json 样式数据
    @objc
    public func applySeatStyleJSONConfig(_ data: Data?) {
        guard let data, !data.isEmpty else {
            kk.SeatCanvasCoreRendererSetSeatStyleJSONConfig(rendererCore, nil, 0)
            return
        }

        #if DEBUG
            print("coreID: \(coreID) SeatStyleJSON: \n\(String(data: data, encoding: .utf8)!)")
        #endif
        data.withUnsafeBytes { buffer in
            kk.SeatCanvasCoreRendererSetSeatStyleJSONConfig(rendererCore, buffer.baseAddress, buffer.count)
        }
    }

    deinit {
        NotificationCenter.default.removeObserver(self, name: UIApplication.didEnterBackgroundNotification, object: nil)
        NotificationCenter.default.removeObserver(self, name: UIApplication.willEnterForegroundNotification, object: nil)

        let coreID = self.coreID
        Task { @MainActor in
            SeatCanvasRendererDelegateRegistry.shared.unregister(coreID: coreID)
        }

        kk.SeatCanvasReleaseCPPObject(self.rendererCore)

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
            print("[SeatCanvasView] 初始化完成, coreID: \(coreID)")
        #endif
    }

    func setupSystemProperties() {
        let density = UIScreen.main.scale
        let fontScale = UIScreen.main.scale
        kk.SeatCanvasInitSystemProperties(density, fontScale)
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
        rendererCore = kk.CreateSeatCanvasCoreRenderer(renderView.layer as? CAEAGLLayer)
        coreID = kk.SeatCanvasCoreRendererGetCoreID(rendererCore)
        if coreID != 0 {
            SeatCanvasRendererDelegateRegistry.shared.register(coreID: coreID, delegate: self)
        }
        backgroundColor = .white
    }

    func setupNotification() {
        NotificationCenter.default.addObserver(self, selector: #selector(appDidEnterBackground(notification:)), name: UIApplication.didEnterBackgroundNotification, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(appWillEnterForeground(notification:)), name: UIApplication.willEnterForegroundNotification, object: nil)
    }

    func rendererInvalidateContent() {
        kk.SeatCanvasCoreRendererInvalidateContent(rendererCore)
    }

    func convertToCanvas(_ point: CGPoint) -> CGPoint {
        let currentZoom = kk.SeatCanvasCoreRendererZoomScale(rendererCore)
        let contentOffset = kk.SeatCanvasCoreRendereContentOffset(rendererCore)
        let density = kk.SeatCanvasCoreRendererGetDensity(rendererCore)

        let px = point.x * density
        let py = point.y * density
        let x = (px - contentOffset.x) / currentZoom
        let y = (py - contentOffset.y) / currentZoom

        return CGPointMake(x, y)
    }

    @objc
    func handleTapGestureRecognizer(gesture: UITapGestureRecognizer) {
        guard gesture.state == .ended else {
            return
        }

        var location = gesture.location(in: renderView)
        let contentScaleFactor = renderView.contentScaleFactor
        location.x *= contentScaleFactor
        location.y *= contentScaleFactor
        kk.SeatCanvasCoreRendererHandTap(rendererCore, location)
//        let location = gesture.location(in: renderView)
//        let locationPx = convertToCanvas(location)
//        var hitTest: kk.HitTestSeatRegionResult = .init()
//        guard kk.SeatCanvasCoreRendererGetSeatRegionByPoint(rendererCore, locationPx, &hitTest), hitTest.valid() else {
//            return
//        }
//
//        print("hitTest: \(hitTest.regionId) \(hitTest.bounds) ")
//        kk.SeatCanvasCoreRendererZoomToRect(rendererCore, hitTest.bounds, true, 20, 300)
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
            kk.SeatCanvasCoreRendererHandPan(rendererCore, kk.gesture.GestureState.BEGAN, translation, timestampMs)
        case .changed:
            kk.SeatCanvasCoreRendererHandPan(rendererCore, kk.gesture.GestureState.CHANGED, translation, timestampMs)
        case .ended, .cancelled:
            kk.SeatCanvasCoreRendererHandPan(rendererCore, (state == .ended) ? kk.gesture.GestureState.ENDED : kk.gesture.GestureState.CANCELLED, translation, timestampMs)
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
            kk.SeatCanvasCoreRendererHandPinch(rendererCore, kk.gesture.GestureState.BEGAN, 1.0, center)
        case .changed:
            let scale = gesture.scale
            kk.SeatCanvasCoreRendererHandPinch(rendererCore, kk.gesture.GestureState.CHANGED, scale, center)
        case .ended, .cancelled:
            let scale = gesture.scale
            kk.SeatCanvasCoreRendererHandPinch(rendererCore, (state == .ended) ? kk.gesture.GestureState.ENDED : kk.gesture.GestureState.CANCELLED, scale, center)
        default:
            break
        }
    }

    func startDrawLoop() {
        kk.SeatCanvasCoreRendererStart(rendererCore)
    }

    func stopDrawLoop() {
        kk.SeatCanvasCoreRendererStop(rendererCore)
    }

    @objc
    func appDidEnterBackground(notification _: Notification) {
        stopDrawLoop()
    }

    @objc
    func appWillEnterForeground(notification _: Notification) {
        startDrawLoop()
    }

    func handleUpdateSize() {
        _ = kk.SeatCanvasCoreRendererUpdateSize(rendererCore)
    }

    func calculateVisibleContentRect() {
        let baseMapScale = kk.SeatCanvasCoreRendererBaseMapScale(rendererCore)

        // C++ 计算
        var cppVisibleContentRect = kk.SeatCanvasCoreRendererGetVisibleContentRect(rendererCore)
        let inverseScale = 1.0 / baseMapScale
        let transform = CGAffineTransform(scaleX: inverseScale, y: inverseScale)
        cppVisibleContentRect = cppVisibleContentRect.applying(transform)
//        print("visibleContentRect: \(cppVisibleContentRect)")
    }
}

// MARK: - C++ 回调处理

extension SeatCanvasView: SeatCanvasRendererDelegate {
    func seatCanvasRendererShouldSelectSeat(regionId: String, seatId: String) -> Bool {
        delegate?.seatCanvasView(self, shouldSelectSeat: regionId, seatId: seatId) ?? false
    }

    func seatCanvasRendererDidSelectSeat(regionId: String, seatId: String) {
        delegate?.seatCanvasView(self, didSelectSeat: regionId, seatId: seatId)
    }

    func seatCanvasRendererDidDeselectSeat(regionId: String, seatId: String) {
        delegate?.seatCanvasView(self, didDeselectSeat: regionId, seatId: seatId)
    }
}
