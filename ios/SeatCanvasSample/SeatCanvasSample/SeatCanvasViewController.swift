//
//  SeatCanvasViewController.swift
//  SeatCanvasSample
//
//  Created by king on 2025/11/12.
//

import SeatCanvas
import UIKit

class SeatCanvasViewController: UIViewController {
    @IBOutlet var containerView: UIView!

    @IBOutlet var circleStyleSwitch: UISwitch!
    @IBOutlet var svgStyleSwitch: UISwitch!

    var seatCanvasView: SeatCanvasView!

    /// 价格: [priceCode: MockPriceData]
    private var prices: [MockPriceData] = []
    /// 座位: [seatId: MockSeatData]
    private var seatsMap: [String: MockSeatData] = [:]
    /// 座位区域: [zoneId: [MockSeatData]]
    private var seatZoneMap: [String: [MockSeatData]] = [:]
    /// 可售的座位: [zoneId: [seatId]]
    private var availableSeats: [String: Set<String>] = [:]
    /// 选中的座位ID
    private var selectedSeatIds: Set<String> = []
    /// 定时刷新可售座位
    private var availableSeatsTimer: Timer?
    private var availableSeatsTimerPaused = false
    private var isViewportInteracting = false
    private let availableSeatsRefreshInterval: TimeInterval = 10.0

    var baseMap: BaseMapFileInfo?
    convenience init(baseMap: BaseMapFileInfo) {
        self.init(nibName: nil, bundle: nil)
        self.baseMap = baseMap
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        seatCanvasView = SeatCanvasView(frame: .zero)
        seatCanvasView.backgroundColor = UIColor(named: "b1")
        seatCanvasView.canvasColor = UIColor(named: "b1")
        seatCanvasView.seatSize = 24
        seatCanvasView.debugHUDEnabled = true
        seatCanvasView.delegate = self
        seatCanvasView.translatesAutoresizingMaskIntoConstraints = false

        containerView.addSubview(seatCanvasView)

        NSLayoutConstraint.activate([
            seatCanvasView.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            seatCanvasView.topAnchor.constraint(equalTo: containerView.topAnchor),
            seatCanvasView.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),
            seatCanvasView.bottomAnchor.constraint(equalTo: containerView.bottomAnchor),

        ])

        loadBaseMap()
    }

    isolated deinit {
        availableSeatsTimer?.invalidate()
    }

    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        stopAvailableSeatsTimer()
    }

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        if availableSeatsTimer == nil {
            startAvailableSeatsTimer()
        }
    }

    @IBAction func handleCircleSeatStyle(_: UISwitch) {
        svgStyleSwitch.isOn = false
        seatCanvasView.applySeatStyleJSONConfig(buildCircleSeatStyleConfig())
    }

    @IBAction func handleSVGSeatStyle(_: UISwitch) {
        circleStyleSwitch.isOn = false
        seatCanvasView.applySeatStyleJSONConfig(buildSVGSeatStyleConfig())
    }

    func loadBaseMap() {
        guard let baseMap else { return }
        guard let data = try? Data(contentsOf: baseMap.url) else {
            return
        }

        let parseConfig = SVGBaseMapParseConfig(zoneIdAttributeNames: ["zoneId", "regioncode"])
        seatCanvasView.loadBaseMap(data, format: .svg, parseConfig: parseConfig)
    }

    func buildCircleSeatStyleConfig() -> Data? {
        let builder = SeatStyleConfigBuilder()
        let disabled = UIColor(named: "seat_disableed")!
        let overlay = UIColor.black.withAlphaComponent(0.7)
        let checkmark = UIColor.white

        guard !prices.isEmpty else {
            return nil
        }

        for price in prices {
            builder.addCircleStyle(styleId: SeatRenderStyleId.compose(pricecode: price.code, status: SeatStatus.unavailable, selected: false), fill: disabled)
            builder.addCircleStyle(styleId: SeatRenderStyleId.compose(pricecode: price.code, status: SeatStatus.available, selected: false), fill: price.color)
            builder.addCircleStyle(styleId: SeatRenderStyleId.compose(pricecode: price.code, status: SeatStatus.available, selected: true), fill: price.color, overlay: overlay, checkmark: checkmark)
        }

        return builder.toJSONData()
    }

    func buildSVGSeatStyleConfig() -> Data? {
        let builder = SeatStyleConfigBuilder()

        guard let availabe = loadSVGContent(name: "icon_seat_selectable.svg"),
              let selected = loadSVGContent(name: "icon_seat_selected.svg"),
              let disabled = loadSVGContent(name: "icon_seat_nonselectable.svg")
        else {
            return nil
        }

        guard !prices.isEmpty else {
            return nil
        }

        for price in prices {
            builder.addSVGStyle(styleId: SeatRenderStyleId.compose(pricecode: price.code, status: SeatStatus.unavailable, selected: false), content: disabled)
            let modifyAvailabe = availabe
                .replacingOccurrences(of: "#EB484A", with: price.color.rgbHex)

            let modifySelected = selected
                .replacingOccurrences(of: "#5BC64D", with: price.color.rgbHex)

            builder.addSVGStyle(styleId: SeatRenderStyleId.compose(pricecode: price.code, status: SeatStatus.available, selected: false), content: modifyAvailabe)
            builder.addSVGStyle(styleId: SeatRenderStyleId.compose(pricecode: price.code, status: SeatStatus.available, selected: true), content: modifySelected)
        }

        return builder.toJSONData()
    }

    func loadSVGContent(name: String) -> String? {
        let url = Bundle.Sample.defaultSeatStyleURL(name: name)
        guard let data = try? Data(contentsOf: url) else {
            return nil
        }

        return String(data: data, encoding: .utf8)
    }

    // MARK: - Mock Data Generation

    func loadMockPrice() -> [MockPriceData] {
        guard let baseMap else {
            return []
        }
        let url = Bundle.Sample.priceDataURL(scope: baseMap.scope, name: baseMap.filename)
        guard let data = try? Data(contentsOf: url) else {
            return []
        }
        do {
            return try JSONDecoder().decode([MockPriceData].self, from: data)
        } catch {
            print(error)
            return []
        }
    }

    func loadMockData() {
        prices = loadMockPrice()
        var zoneColors: [String: UIColor] = [:]
        for price in prices {
            for zoneId in price.zoneIds {
                zoneColors[zoneId] = price.color
            }
        }

        seatCanvasView.updateSeatZoneAlternateColors(colors: zoneColors)
        seatCanvasView.updateMiniMapZoneAlternateColors(colors: zoneColors)

        seatCanvasView.registerPricecodes(prices.map(\.code))

        seatsMap.removeAll(keepingCapacity: true)
        seatZoneMap = loadSeatDatas()
        selectedSeatIds.removeAll(keepingCapacity: true)
        for zoneSeat in seatZoneMap {
            for seat in zoneSeat.value {
                seatsMap[seat.seatId] = seat
                if seat.selected {
                    selectedSeatIds.insert(seat.seatId)
                }
            }
            let seatDatas = zoneSeat.value.map {
                SeatData(
                    seatId: $0.seatId,
                    position: $0.position,
                    rotation: $0.rotation,
                    pricecode: $0.pricecode.isEmpty ? nil : $0.pricecode
                )
            }
            seatCanvasView.updateSeatDatas(zoneId: zoneSeat.key, seats: seatDatas)
            seatCanvasView.updateSeatStatusesForZone(
                zoneId: zoneSeat.key,
                statuses: buildStatusesForZone(zoneId: zoneSeat.key, seats: zoneSeat.value)
            )
        }
        seatCanvasView.setSelectedSeatIds(Array(selectedSeatIds))
    }

    private enum SeatStatus {
        static let unavailable: UInt32 = 0
        static let available: UInt32 = 1
    }

    private func buildStatusesForZone(zoneId: String, seats: [MockSeatData]) -> [NSNumber] {
        seats.map { seat in
            NSNumber(value: seatAvailable(zoneId: zoneId, seatId: seat.seatId) ? SeatStatus.available : SeatStatus.unavailable)
        }
    }

    private func pushSelectedSeatIds(previousSelected: Set<String>) {
        let added = selectedSeatIds.filter { !previousSelected.contains($0) }
        let removed = previousSelected.filter { !selectedSeatIds.contains($0) }
        seatCanvasView.updateSelectedSeatIds(added: Array(added), removed: Array(removed))
    }

    func loadZoneDatas() -> [MockZoneInfo] {
        guard let baseMap else {
            return []
        }
        let url = Bundle.Sample.zoneDataURL(scope: baseMap.scope, name: baseMap.filename)
        guard let data = try? Data(contentsOf: url) else {
            return []
        }
        do {
            return try JSONDecoder().decode([MockZoneInfo].self, from: data)
        } catch {
            print(error)
            return []
        }
    }

    func loadSeatDatas() -> [String: [MockSeatData]] {
        guard let baseMap else {
            return [:]
        }
        let url = Bundle.Sample.seatDataURL(scope: baseMap.scope, name: baseMap.filename)
        guard let data = try? Data(contentsOf: url) else {
            return [:]
        }
        do {
            return try JSONDecoder().decode([String: [MockSeatData]].self, from: data)
        } catch {
            print(error)
            return [:]
        }
    }

    private func seatAvailable(zoneId: String, seatId: String) -> Bool {
        guard let seats = availableSeats[zoneId] else {
            return false
        }
        return seats.contains(seatId)
    }

    private func startAvailableSeatsTimer() {
        availableSeatsTimer?.invalidate()
        availableSeatsTimerPaused = false
        availableSeatsTimer = Timer.scheduledTimer(
            withTimeInterval: availableSeatsRefreshInterval,
            repeats: true
        ) { [weak self] _ in
            guard let self, !self.availableSeatsTimerPaused else {
                return
            }
            self.regenerateRandomAvailableSeats(fullRefresh: false)
        }
    }

    private func pauseAvailableSeatsTimer() {
        availableSeatsTimerPaused = true
    }

    private func resumeAvailableSeatsTimer() {
        guard availableSeatsTimerPaused else {
            return
        }
        availableSeatsTimerPaused = false
    }

    private func stopAvailableSeatsTimer() {
        availableSeatsTimerPaused = false
        isViewportInteracting = false
        availableSeatsTimer?.invalidate()
        availableSeatsTimer = nil
    }

    private func beginViewportInteraction() {
        if isViewportInteracting {
            return
        }
        isViewportInteracting = true
        pauseAvailableSeatsTimer()
    }

    private func endViewportInteraction() {
        if !isViewportInteracting {
            return
        }
        isViewportInteracting = false
        regenerateRandomAvailableSeats(fullRefresh: false)
        resumeAvailableSeatsTimer()
    }

    private func regenerateRandomAvailableSeats(fullRefresh: Bool = false) {
        let zoneIds: [String]
        if fullRefresh {
            zoneIds = Array(seatZoneMap.keys)
        } else {
            let zoomScale = seatCanvasView.zoomScale
            let seatRenderZoomThreshold = seatCanvasView.seatRenderZoomThreshold
            guard zoomScale >= seatRenderZoomThreshold else {
                return
            }
            let visibleRect = seatCanvasView.visibleOriginalRect()
            let visibleZoneIds = seatCanvasView.zoneIds(inOriginalRect: visibleRect)
            guard !visibleZoneIds.isEmpty else {
                return
            }
            zoneIds = visibleZoneIds
        }

        if fullRefresh {
            availableSeats.removeAll(keepingCapacity: true)
        }

        for zoneId in zoneIds {
            guard let seats = seatZoneMap[zoneId], !seats.isEmpty else {
                continue
            }
            let ratio = Double.random(in: 0.2 ... 0.9)
            let availableCount = max(1, Int(Double(seats.count) * ratio))
            let availableIds = Set(seats.shuffled().prefix(availableCount).map(\.seatId))
            availableSeats[zoneId] = availableIds
            seatCanvasView.updateSeatStatusesForZone(
                zoneId: zoneId,
                statuses: buildStatusesForZone(zoneId: zoneId, seats: seats)
            )
        }
        pruneSelectedSeatsForAvailability()
        seatCanvasView.setSelectedSeatIds(Array(selectedSeatIds))

        if !fullRefresh {
            print("refreshAvailableSeatsForVisibleZones: zoneIds=\(zoneIds)")
        }
    }

    private func pruneSelectedSeatsForAvailability() {
        selectedSeatIds = selectedSeatIds.filter { seatId in
            for (zoneId, seats) in seatZoneMap {
                guard seats.contains(where: { $0.seatId == seatId }) else {
                    continue
                }
                return seatAvailable(zoneId: zoneId, seatId: seatId)
            }
            return false
        }
    }
}

extension SeatCanvasViewController: SeatCanvasViewDelegate {
    /// 底图加载完成；此时 ZoomLevelConfig 与初始缩放已就绪，可设置 seatRenderZoomThreshold 并 push 座位数据。
    func seatCanvasView(_ view: SeatCanvasView, didLoadBaseMap event: SeatCanvasBaseMapLoadedEvent) {
        view.seatRenderZoomThreshold = event.zoomLevels.venue
        loadMockData()
        regenerateRandomAvailableSeats(fullRefresh: true)
        view.applySeatStyleJSONConfig(buildSVGSeatStyleConfig())
        if availableSeatsTimer == nil {
            startAvailableSeatsTimer()
        }
    }

    func seatCanvasViewDidUnloadBaseMap(_: SeatCanvasView) {
        stopAvailableSeatsTimer()
        availableSeats.removeAll()
        selectedSeatIds.removeAll()
    }

    /// 点击某个座位，业务层处理状态变更。
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID
    /// - Returns: 是否发生了状态变化。true 则触发重绘。
    func seatCanvasView(_: SeatCanvasView, didTapSeat zoneId: String, seatId: String) -> Bool {
        let previousSelected = selectedSeatIds
        var changed = false
        if selectedSeatIds.contains(seatId) {
            selectedSeatIds.remove(seatId)
            changed = true
        } else if seatAvailable(zoneId: zoneId, seatId: seatId) {
            selectedSeatIds.insert(seatId)
            changed = true
        }
        if changed {
            pushSelectedSeatIds(previousSelected: previousSelected)
        }
        print(#function, zoneId, seatId)
        return changed
    }

    /// 点击某个区域
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    func seatCanvasView(_: SeatCanvasView, didTapZone zoneId: String) {
        print(#function, zoneId)
    }

    /// 即将开始拖动视图
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewWillBeginDragging(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
        print(#function)
        beginViewportInteraction()
    }

    /// 视图发生滚动
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewDidScroll(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
//        print(#function)
    }

    /// 拖动手势结束
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    ///   - decelerate: 是否会继续惯性滚动或回弹动画
    func seatCanvasViewDidEndDragging(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect, decelerate: Bool) {
        print(#function, decelerate)
        if !decelerate {
            endViewportInteraction()
        }
    }

    /// 惯性滚动或回弹动画结束
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewDidEndDecelerating(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
        print(#function)
        endViewportInteraction()
    }

    /// 即将开始缩放视图
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewWillBeginZooming(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
        print(#function)
        beginViewportInteraction()
    }

    /// 视图发生缩放
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewDidZoom(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
        print(#function)
    }

    /// 缩放手势结束
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewDidEndZooming(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
        print(#function)
        endViewportInteraction()
    }

    /// 程序触发的滚动或缩放动画结束
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewDidEndScrollingAnimation(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
        print(#function)
        endViewportInteraction()
    }
}
