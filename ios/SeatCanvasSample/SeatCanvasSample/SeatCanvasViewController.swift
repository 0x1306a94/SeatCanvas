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

        seatCanvasView.loadBaseMap(data, format: .svg)
    }

    func buildCircleSeatStyleConfig() -> Data? {
        let builder = SeatStyleConfigBuilder()
        let availabe = UIColor(named: "seat_available")!
        let sold = UIColor(named: "seat_sold")!
        let locked = UIColor(named: "seat_locked")!
        let disabled = UIColor(named: "seat_disableed")!
        let overlay = UIColor.black.withAlphaComponent(0.7)
        let checkmark = UIColor.white
        builder.addCircleStyle(status: 0, selected: false, fill: availabe, overlay: overlay, checkmark: checkmark)
        builder.addCircleStyle(status: 0, selected: true, fill: availabe, overlay: overlay, checkmark: checkmark)

        builder.addCircleStyle(status: 1, selected: false, fill: sold, overlay: overlay, checkmark: checkmark)
        builder.addCircleStyle(status: 1, selected: true, fill: sold, overlay: overlay, checkmark: checkmark)

        builder.addCircleStyle(status: 2, selected: false, fill: locked, overlay: overlay, checkmark: checkmark)
        builder.addCircleStyle(status: 2, selected: true, fill: locked, overlay: overlay, checkmark: checkmark)

        builder.addCircleStyle(status: 3, selected: false, fill: disabled, overlay: overlay, checkmark: checkmark)
        builder.addCircleStyle(status: 3, selected: true, fill: disabled, overlay: overlay, checkmark: checkmark)

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

        builder.addSVGStyle(status: 0, selected: false, content: availabe)
        builder.addSVGStyle(status: 0, selected: true, content: selected)

        builder.addSVGStyle(status: 1, selected: false, content: disabled)
        builder.addSVGStyle(status: 1, selected: true, content: disabled)

        builder.addSVGStyle(status: 2, selected: false, content: disabled)
        builder.addSVGStyle(status: 2, selected: true, content: disabled)

        builder.addSVGStyle(status: 3, selected: false, content: disabled)
        builder.addSVGStyle(status: 3, selected: true, content: disabled)

        return builder.toJSONData()
    }

    func loadSVGContent(name: String) -> String? {
        let url = Bundle.Sample.defaultSeatStyleURL(name: name)
        guard let data = try? Data(contentsOf: url) else {
            return nil
        }

        let content = String(data: data, encoding: .utf8)
        return content
    }

    // MARK: - Mock Data Generation

    func loadMockData() {
        let zoneDatas = loadZoneDatas()
        seatCanvasView.updateSeatZoneDatas(zones: zoneDatas)

        let seats = loadSeatDatas()
        for seat in seats {
            seatCanvasView.updateSeatDatas(zoneId: seat.key, seats: seat.value.map { SeatData(seatId: $0.seatId, status: $0.status, selected: $0.selected, position: $0.position) })
        }
    }

    func loadZoneDatas() -> [SeatZoneData] {
        guard let baseMap else {
            return []
        }
        let url = Bundle.Sample.zoneDataURL(scope: baseMap.scope, name: baseMap.filename)
        guard let data = try? Data(contentsOf: url) else {
            return []
        }
        do {
            let zones = try JSONDecoder().decode([MockZoneInfo].self, from: data)
            return zones.map { SeatZoneData(zoneId: $0.zoneId, color: $0.color, pirceColor: $0.pirceColor) }
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
            let seats = try JSONDecoder().decode([String: [MockSeatData]].self, from: data)
            return seats
        } catch {
            print(error)
            return [:]
        }
    }
}

extension SeatCanvasViewController: SeatCanvasViewDelegate {
    /// 是否可以选中座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID
    func seatCanvasView(_: SeatCanvasView, shouldSelectSeat _: String, seatId _: String) -> Bool {
        true
    }

    /// 选中某个座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID
    func seatCanvasView(_: SeatCanvasView, didSelectSeat zoneId: String, seatId: String) {
        print(#function, zoneId, seatId)
    }

    /// 取消选中某个座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID
    func seatCanvasView(_: SeatCanvasView, didDeselectSeat zoneId: String, seatId: String) {
        print(#function, zoneId, seatId)
    }
}
