//
//  SeatCanvasView.swift
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

import UIKit

internal import SeatCanvas_Private

@MainActor
@objc(KKSeatCanvasView)
public class SeatCanvasView: UIView {
    var backendView: SeatCanvasBackendView!
    var zoomLevel: kk.ZoomLevel = .init(zoomScale9: 1.0, zoomScale18: 1.0, zoomScale30: 1.0, zoomScale50: 1.0)
    var isSmallVenue = false
    var minimapImage: UIImage?

    nonisolated(unsafe) let rendererCore: UnsafeMutablePointer<kk.CPPObject> = kk.CreateSeatCanvasCoreRenderer(nil)
    nonisolated(unsafe) var coreID: UInt32 = 0

    /// 座位选中事件代理
    @objc
    public weak var delegate: SeatCanvasViewDelegate?

    var tapGestureRecognizer: UITapGestureRecognizer!
    var panGestureRecognizer: UIPanGestureRecognizer!
    var pinchGestureRecognizer: UIPinchGestureRecognizer!

    @objc
    public var enableTiled: Bool = false {
        didSet {
            kk.SeatCanvasCoreRendererEnableTiled(rendererCore, enableTiled)
        }
    }

    @objc
    public var enableZoomBlur: Bool = false {
        didSet {
            kk.SeatCanvasCoreRendererEnableZoomBlur(rendererCore, enableZoomBlur)
        }
    }

    override public var backgroundColor: UIColor? {
        didSet {
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

    public func loadBaseMap(_ data: Data?) {
        guard let data else {
            kk.SeatCanvasCoreRendererLoadBaseMap(rendererCore, nil)
            return
        }

        DispatchQueue.global(qos: .userInteractive).async {
            var minimapImage: UIImage? = nil
            var loadResult = data.withUnsafeBytes { buffer in
                let result = kk.SeatCanvasLoadBaseMapFromSVG(buffer.baseAddress, buffer.count, &minimapImage)
                return result
            }

            DispatchQueue.main.async { [weak self] in
                guard let self else { return }
                self.minimapImage = minimapImage
                kk.SeatCanvasCoreRendererLoadBaseMap(self.rendererCore, &loadResult)
            }
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
        backgroundColor = .white

        setupViews()
        setupGestureRecognizer()
        setupRenderer()
        setupNotification()

        #if DEBUG
            print("[SeatCanvasView] 初始化完成, coreID: \(coreID)")
        #endif
    }

    func setupViews() {
        backendView = SeatCanvasBackendView(frame: bounds)
        backendView.translatesAutoresizingMaskIntoConstraints = false
        backendView.contentScaleFactor = UIScreen.main.scale
        backendView.didUpdateSize = { [weak self] _ in
            self?.handleUpdateSize()
        }

        addSubview(backendView)

        NSLayoutConstraint.activate([
            backendView.leadingAnchor.constraint(equalTo: leadingAnchor),
            backendView.topAnchor.constraint(equalTo: topAnchor),
            backendView.trailingAnchor.constraint(equalTo: trailingAnchor),
            backendView.bottomAnchor.constraint(equalTo: bottomAnchor),
        ])
    }

    func setupGestureRecognizer() {
        tapGestureRecognizer = UITapGestureRecognizer(target: self, action: #selector(handleTapGestureRecognizer(gesture:)))

        panGestureRecognizer = UIPanGestureRecognizer(target: self, action: #selector(handlePanGestureRecognizer(gesture:)))
        pinchGestureRecognizer = UIPinchGestureRecognizer(target: self, action: #selector(handlePinchGestureRecognizer(gesture:)))

        backendView.addGestureRecognizer(panGestureRecognizer)
        backendView.addGestureRecognizer(pinchGestureRecognizer)

        tapGestureRecognizer.require(toFail: panGestureRecognizer)

        backendView.addGestureRecognizer(tapGestureRecognizer)
    }

    func setupRenderer() {
        kk.SeatCanvasCoreRendererReplaceBackend(rendererCore, backendView.layer as? CAEAGLLayer)
        coreID = kk.SeatCanvasCoreRendererGetCoreID(rendererCore)
        if coreID != 0 {
            SeatCanvasRendererDelegateRegistry.shared.register(coreID: coreID, delegate: self)
        }
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

        var location = gesture.location(in: backendView)
        let contentScaleFactor = backendView.contentScaleFactor
        location.x *= contentScaleFactor
        location.y *= contentScaleFactor
        kk.SeatCanvasCoreRendererHandTap(rendererCore, location)
//        let location = gesture.location(in: backendView)
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

        var translation = gesture.translation(in: backendView)
        let contentScaleFactor = -backendView.contentScaleFactor
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
            gesture.setTranslation(.zero, in: backendView)
        default:
            break
        }
    }

    @objc
    func handlePinchGestureRecognizer(gesture: UIPinchGestureRecognizer) {
        guard gesture.state != .possible, gesture.state != .failed else {
            return
        }

        var center = gesture.location(in: backendView)
        let contentScaleFactor = backendView.contentScaleFactor
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
    func seatCanvasRendererShouldSelectSeat(seatId: String) -> Bool {
        delegate?.seatCanvasView(self, shouldSelectSeat: seatId) ?? false
    }

    func seatCanvasRendererDidSelectSeat(seatId: String) {
        delegate?.seatCanvasView(self, didSelectSeat: seatId)
    }

    func seatCanvasRendererDidDeselectSeat(seatId: String) {
        delegate?.seatCanvasView(self, didDeselectSeat: seatId)
    }
}
