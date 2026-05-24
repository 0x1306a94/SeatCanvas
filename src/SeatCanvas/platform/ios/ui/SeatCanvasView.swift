//
//  SeatCanvasView.swift
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

import Foundation
import Metal
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
            var clearColor = MTLClearColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 0.0)
            if let newValue {
                var r: CGFloat = 0
                var g: CGFloat = 0
                var b: CGFloat = 0
                var a: CGFloat = 0
                if newValue.getRed(&r, green: &g, blue: &b, alpha: &a) {
                    clearColor = MTLClearColor(red: r, green: g, blue: b, alpha: a)
                }
            }
            renderView.clearColor = clearColor
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

    /// Temporary warmup entry for shader compiler initialization.
    /// TODO: Remove this API after warmup is integrated into renderer startup flow.
    @objc
    public nonisolated static func prewarmShaderCompiler() {
        SeatCanvasCoreRenderer.prewarmShaderCompiler()
    }

    /// 加载底图（指定格式和解析配置）
    /// - Parameters:
    ///   - data: 底图数据
    ///   - format: 格式
    ///   - parseConfig: 解析配置
    @objc
    public func loadBaseMap(_ data: Data?, format: BaseMapFormat, parseConfig: BaseMapParseConfig? = nil) {
        renderer?.loadBaseMap(data, format: format, parseConfigJSON: parseConfig?.serializeToData())
    }

    /// 应用样式配置（使用 SeatStyleConfigBuilder 构建）
    /// - Parameter data: json 样式数据
    @objc
    public func applySeatStyleJSONConfig(_ data: Data?) {
        renderer?.applySeatStyleJSONConfig(data)
    }

    /// 更新区域颜色
    /// - Parameter colors: 颜色表 (key 区域ID， value 颜色)
    @objc
    public func updateSeatZoneAlternateColors(colors: [String: UIColor]?) {
        renderer?.updateSeatZoneAlternateColors(colors: colors)
    }

    /// 更新小地图区域颜色
    /// - Parameter colors: 颜色表 (key 区域ID， value 颜色)
    @objc
    public func updateMiniMapZoneAlternateColors(colors: [String: UIColor]?) {
        renderer?.updateMiniMapZoneAlternateColors(colors: colors)
    }

    /// 更新区域座位几何数据；会重置该 zone 的 status 为 0，并清除旧 seat 的 selected，随后需 re-push status/selected。
    /// - Parameters:
    ///   - zoneId: 区域ID
    ///   - seats: 座位集合
    @objc
    public func updateSeatDatas(zoneId: String, seats: [SeatData]) {
        renderer?.updateSeatDatas(zoneId: zoneId, seats: seats)
    }

    /// 清除全部座位数据
    @objc
    public func clearSeatData() {
        renderer?.clearSeatData()
    }

    /// 注册价档表
    @objc
    public func registerPricecodes(_ pricecodes: [String]) {
        renderer?.registerPricecodes(pricecodes)
    }

    /// 批量更新单个座位 status
    @objc
    public func updateSeatStatuses(seatIds: [String], statuses: [NSNumber]) {
        let values = statuses.map { $0.uint32Value }
        renderer?.updateSeatStatuses(seatIds: seatIds, statuses: values)
    }

    /// 批量更新某个 zone 内全部座位 status（raw uint32 binary data，数组下标与 updateSeatDatas 顺序一致）
    @objc
    public func updateSeatStatusesForZone(zoneId: String, statusData: Data) {
        renderer?.updateSeatStatusesForZone(zoneId: zoneId, statusData: statusData)
    }

    /// 全量替换选中座位
    @objc
    public func setSelectedSeatIds(_ seatIds: [String]) {
        renderer?.setSelectedSeatIds(seatIds)
    }

    /// 增量更新选中座位
    @objc
    public func updateSelectedSeatIds(added: [String], removed: [String]) {
        renderer?.updateSelectedSeatIds(added: added, removed: removed)
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

    /// 是否绘制调试 HUD（FPS、缩放级别、座位统计等），默认关闭
    @objc
    public var debugHUDEnabled: Bool {
        get {
            renderer?.debugHUDEnabled ?? false
        }
        set {
            renderer?.debugHUDEnabled = newValue
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

    /// 当前缩放级别
    @objc
    public var zoomScale: CGFloat {
        renderer?.zoomScale ?? 1.0
    }

    /// 最大缩放级别
    @objc
    public var maximumZoomScale: CGFloat {
        renderer?.maximumZoomScale ?? 1.0
    }

    /// 获取当前显示范围（原始坐标系）
    @objc
    public func visibleOriginalRect() -> CGRect {
        renderer?.visibleOriginalRect() ?? .zero
    }

    /// 查找与指定矩形相交的区域 ID 列表（原始坐标系）
    /// - Parameter rect: 查询矩形，通常配合 visibleOriginalRect() 使用
    /// - Returns: 相交区域的 zoneId 列表
    @objc
    public func zoneIds(inOriginalRect rect: CGRect) -> [String] {
        renderer?.zoneIds(inOriginalRect: rect) ?? []
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
        renderView = SeatCanvasRenderView(frame: bounds, device: MTLCreateSystemDefaultDevice())
        renderView.backgroundColor = .clear
        renderView.clearColor = MTLClearColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 0.0)
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
        renderer = SeatCanvasCoreRenderer(metalView: renderView)
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
    func seatCanvasRendererDidLoadBaseMap() {
        delegate?.seatCanvasViewDidLoadBaseMap?(self)
    }

    func seatCanvasRendererDidUnloadBaseMap() {
        delegate?.seatCanvasViewDidUnloadBaseMap?(self)
    }

    func seatCanvasRendererDidUpdateZoomLevelConfig(zoomLevels: ZoomLevel, minimumZoomScale: CGFloat, maximumZoomScale: CGFloat, zoomScale: CGFloat) {
        delegate?.seatCanvasView?(self, didUpdateZoomLevelConfig: zoomLevels, minimumZoomScale: minimumZoomScale, maximumZoomScale: maximumZoomScale, zoomScale: zoomScale)
    }

    func seatCanvasRendererDidTapZone(zoneId: String) {
        delegate?.seatCanvasView(self, didTapZone: zoneId)
    }

    func seatCanvasRendererDidTapSeat(zoneId: String, seatId: String) -> Bool {
        delegate?.seatCanvasView(self, didTapSeat: zoneId, seatId: seatId) ?? false
    }

    func seatCanvasRendererWillBeginDragging(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect) {
        delegate?.seatCanvasViewWillBeginDragging?(self, zoomScale: zoomScale, contentOffset: contentOffset, visibleOriginalRect: visibleOriginalRect)
    }

    func seatCanvasRendererDidScroll(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect) {
        delegate?.seatCanvasViewDidScroll?(self, zoomScale: zoomScale, contentOffset: contentOffset, visibleOriginalRect: visibleOriginalRect)
    }

    func seatCanvasRendererDidEndDragging(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect, decelerate: Bool) {
        delegate?.seatCanvasViewDidEndDragging?(self, zoomScale: zoomScale, contentOffset: contentOffset, visibleOriginalRect: visibleOriginalRect, decelerate: decelerate)
    }

    func seatCanvasRendererDidEndDecelerating(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect) {
        delegate?.seatCanvasViewDidEndDecelerating?(self, zoomScale: zoomScale, contentOffset: contentOffset, visibleOriginalRect: visibleOriginalRect)
    }

    func seatCanvasRendererWillBeginZooming(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect) {
        delegate?.seatCanvasViewWillBeginZooming?(self, zoomScale: zoomScale, contentOffset: contentOffset, visibleOriginalRect: visibleOriginalRect)
    }

    func seatCanvasRendererDidZoom(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect) {
        delegate?.seatCanvasViewDidZoom?(self, zoomScale: zoomScale, contentOffset: contentOffset, visibleOriginalRect: visibleOriginalRect)
    }

    func seatCanvasRendererDidEndZooming(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect) {
        delegate?.seatCanvasViewDidEndZooming?(self, zoomScale: zoomScale, contentOffset: contentOffset, visibleOriginalRect: visibleOriginalRect)
    }

    func seatCanvasRendererDidEndScrollingAnimation(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect) {
        delegate?.seatCanvasViewDidEndScrollingAnimation?(self, zoomScale: zoomScale, contentOffset: contentOffset, visibleOriginalRect: visibleOriginalRect)
    }
}
