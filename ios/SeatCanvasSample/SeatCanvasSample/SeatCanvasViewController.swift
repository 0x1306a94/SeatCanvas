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

    private var seatStatusMap: [String: UInt32] = [:]
    private var selectedSeatIds: Set<String> = []

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
        seatCanvasView.delegate = self
        seatCanvasView.translatesAutoresizingMaskIntoConstraints = false

        containerView.addSubview(seatCanvasView)

        NSLayoutConstraint.activate([
            seatCanvasView.leadingAnchor.constraint(equalTo: containerView.leadingAnchor),
            seatCanvasView.topAnchor.constraint(equalTo: containerView.topAnchor),
            seatCanvasView.trailingAnchor.constraint(equalTo: containerView.trailingAnchor),
            seatCanvasView.bottomAnchor.constraint(equalTo: containerView.bottomAnchor),

        ])

        seatCanvasView.applySeatStyleJSONConfig(buildSVGSeatStyleConfig())

        loadBaseMap()
        loadMockData()
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
        let availabe = UIColor(named: "seat_available")!
        let sold = UIColor(named: "seat_sold")!
        let locked = UIColor(named: "seat_locked")!
        let disabled = UIColor(named: "seat_disableed")!
        let overlay = UIColor.black.withAlphaComponent(0.7)
        let checkmark = UIColor.white

        builder.addCircleStyle(styleId: styleId(status: 0, selected: false), fill: availabe)
        builder.addCircleStyle(styleId: styleId(status: 0, selected: true), fill: availabe, overlay: overlay, checkmark: checkmark)

        builder.addCircleStyle(styleId: styleId(status: 1, selected: false), fill: sold)
        builder.addCircleStyle(styleId: styleId(status: 1, selected: true), fill: sold, overlay: overlay, checkmark: checkmark)

        builder.addCircleStyle(styleId: styleId(status: 2, selected: false), fill: locked)
        builder.addCircleStyle(styleId: styleId(status: 2, selected: true), fill: locked, overlay: overlay, checkmark: checkmark)

        builder.addCircleStyle(styleId: styleId(status: 3, selected: false), fill: disabled)
        builder.addCircleStyle(styleId: styleId(status: 3, selected: true), fill: disabled, overlay: overlay, checkmark: checkmark)

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

        builder.addSVGStyle(styleId: styleId(status: 0, selected: false), content: availabe)
        builder.addSVGStyle(styleId: styleId(status: 0, selected: true), content: selected)

        builder.addSVGStyle(styleId: styleId(status: 1, selected: false), content: disabled)
        builder.addSVGStyle(styleId: styleId(status: 1, selected: true), content: disabled)

        builder.addSVGStyle(styleId: styleId(status: 2, selected: false), content: disabled)
        builder.addSVGStyle(styleId: styleId(status: 2, selected: true), content: disabled)

        builder.addSVGStyle(styleId: styleId(status: 3, selected: false), content: disabled)
        builder.addSVGStyle(styleId: styleId(status: 3, selected: true), content: disabled)

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

    func loadMockData() {
        let zoneDatas = loadZoneDatas()
        let zoneColors = zoneDatas.reduce(into: [String: UIColor]()) { $0[$1.zoneId] = $1.alternateColor }
        seatCanvasView.updateSeatZoneAlternateColors(colors: zoneColors)
        seatCanvasView.updateMiniMapZoneAlternateColors(colors: zoneColors)

        let seats = loadSeatDatas()
        for zoneSeat in seats {
            for seat in zoneSeat.value {
                seatStatusMap[seat.seatId] = seat.status
                if seat.selected {
                    selectedSeatIds.insert(seat.seatId)
                }
            }
            seatCanvasView.updateSeatDatas(zoneId: zoneSeat.key, seats: zoneSeat.value.map { SeatData(seatId: $0.seatId, position: $0.position, rotation: $0.rotation) })
        }
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

    private func styleId(status: UInt32, selected: Bool) -> String {
        "status_\(status)_selected_\(selected ? 1 : 0)"
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
}

extension SeatCanvasViewController: SeatCanvasViewDelegate {
    /// 根据区域和座位ID返回样式ID。
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID（由业务保证全局唯一）
    /// - Returns: 样式ID。返回 nil 或空字符串表示该座位不渲染。
    func seatCanvasView(_: SeatCanvasView, styleIdForSeat _: String, seatId: String) -> String? {
        guard let status = seatStatusMap[seatId] else {
            return nil
        }
        return styleId(status: status, selected: selectedSeatIds.contains(seatId))
    }

    /// 点击某个座位，业务层处理状态变更。
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID
    /// - Returns: 是否发生了状态变化。true 则触发重绘。
    func seatCanvasView(_: SeatCanvasView, didTapSeat zoneId: String, seatId: String) -> Bool {
        if selectedSeatIds.contains(seatId) {
            selectedSeatIds.remove(seatId)
        } else {
            selectedSeatIds.insert(seatId)
        }
        print(#function, zoneId, seatId)
        return true
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
    }

    /// 视图发生滚动
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewDidScroll(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
        print(#function)
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
    }

    /// 惯性滚动或回弹动画结束
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewDidEndDecelerating(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
        print(#function)
    }

    /// 即将开始缩放视图
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewWillBeginZooming(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
        print(#function)
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
    }

    /// 程序触发的滚动或缩放动画结束
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    func seatCanvasViewDidEndScrollingAnimation(_: SeatCanvasView, zoomScale _: CGFloat, contentOffset _: CGPoint, visibleOriginalRect _: CGRect) {
        print(#function)
    }
}
