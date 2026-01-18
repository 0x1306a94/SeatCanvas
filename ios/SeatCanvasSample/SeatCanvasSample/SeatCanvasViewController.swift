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

    var basemapName: String?
    lazy var baseMapBundle: Bundle = {
        let path = Bundle.main.path(forResource: "SVGBaseMap", ofType: "bundle")!
        let bundle = Bundle(path: path)!
        return bundle
    }()

    convenience init(basemapName: String) {
        self.init(nibName: nil, bundle: nil)
        self.basemapName = basemapName
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view.

        seatCanvasView = SeatCanvasView(frame: .zero)
        seatCanvasView.backgroundColor = UIColor(named: "b1")
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

        // 加载底图后，生成并设置 mock 数据
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) { [weak self] in
            self?.loadMockData()
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
        guard let basemapName else { return }
        guard let svgPath = baseMapBundle.path(forResource: basemapName, ofType: "svg") else {
            return
        }

        guard let data = try? Data(contentsOf: URL(fileURLWithPath: svgPath)) else {
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

        guard let availabe = loadSVGContent(name: "icon_seat_selectable"),
              let selected = loadSVGContent(name: "icon_seat_selected"),
              let disabled = loadSVGContent(name: "icon_seat_nonselectable")
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
        guard let svgPath = baseMapBundle.path(forResource: name, ofType: "svg") else {
            return nil
        }

        guard let data = try? Data(contentsOf: URL(fileURLWithPath: svgPath)) else {
            return nil
        }

        let content = String(data: data, encoding: .utf8)
        return content
    }

    // MARK: - Mock Data Generation

    func loadMockData() {
        let zones = generateMockZoneDatas()
        seatCanvasView.updateSeatZoneDatas(zones: zones)

        let regions = getMockRegions()
        for region in regions {
            let seats = generateMockSeatDatas(rect: region.rect)
            seatCanvasView.updateSeatDatas(zoneId: region.zoneId, seats: seats)
        }
    }

    struct RegionInfo {
        let zoneId: String
        let rect: CGRect
    }

    func getMockRegions() -> [RegionInfo] {
        return [
            RegionInfo(zoneId: "20011", rect: CGRect(x: 850, y: 2800, width: 550, height: 320)),
            RegionInfo(zoneId: "20012", rect: CGRect(x: 850, y: 3170, width: 550, height: 320)),
            RegionInfo(zoneId: "20013", rect: CGRect(x: 850, y: 3540, width: 550, height: 320)),
            RegionInfo(zoneId: "20014", rect: CGRect(x: 850, y: 3910, width: 550, height: 320)),
            RegionInfo(zoneId: "20031", rect: CGRect(x: 2000, y: 2800, width: 450, height: 280)),
            RegionInfo(zoneId: "20032", rect: CGRect(x: 2000, y: 3130, width: 450, height: 280)),
            RegionInfo(zoneId: "20034", rect: CGRect(x: 2000, y: 3790, width: 450, height: 280)),
            RegionInfo(zoneId: "10085", rect: CGRect(x: 1700, y: 1520, width: 700, height: 230)),
            RegionInfo(zoneId: "10086", rect: CGRect(x: 2450, y: 1520, width: 700, height: 230)),
            RegionInfo(zoneId: "30031", rect: CGRect(x: 9550, y: 2800, width: 450, height: 280)),
            RegionInfo(zoneId: "30032", rect: CGRect(x: 9550, y: 3130, width: 450, height: 280)),
            RegionInfo(zoneId: "30033", rect: CGRect(x: 9550, y: 3460, width: 450, height: 280)),
            RegionInfo(zoneId: "10096", rect: CGRect(x: 9600, y: 1800, width: 700, height: 220)),
            RegionInfo(zoneId: "10098", rect: CGRect(x: 11100, y: 1800, width: 700, height: 220)),
            RegionInfo(zoneId: "40109", rect: CGRect(x: 6950, y: 9300, width: 700, height: 250)),
            RegionInfo(zoneId: "40110", rect: CGRect(x: 7700, y: 9300, width: 700, height: 250)),
            RegionInfo(zoneId: "40123", rect: CGRect(x: 6600, y: 9600, width: 750, height: 300)),
            RegionInfo(zoneId: "40124", rect: CGRect(x: 7400, y: 9600, width: 750, height: 300)),
            RegionInfo(zoneId: "37492", rect: CGRect(x: 33, y: 411, width: 289, height: 329)),
            RegionInfo(zoneId: "74148", rect: CGRect(x: 400, y: 76, width: 332, height: 232)),
        ]
    }

    func generateMockZoneDatas() -> [SeatZoneData] {
        let regions = getMockRegions()
        return regions.map { region in
            let color = UIColor.white
            let priceColor = UIColor(
                red: CGFloat.random(in: 0.5 ... 1.0),
                green: CGFloat.random(in: 0.5 ... 1.0),
                blue: CGFloat.random(in: 0.5 ... 1.0),
                alpha: 1.0
            )
            return SeatZoneData(zoneId: region.zoneId, color: color, pirceColor: priceColor)
        }
    }

    func generateMockSeatDatas(rect: CGRect) -> [SeatData] {
        let itemSize: CGFloat = 36.0
        let spacing: CGFloat = 10.0

        // 计算每行和每列能放多少个座位
        let cols = Int((rect.width) / (itemSize + spacing))
        let rows = Int((rect.height) / (itemSize + spacing))

        var seats: [SeatData] = []

        // 生成网格状的座位数据
        for row in 0 ..< rows {
            for col in 0 ..< cols {
                let seatX = rect.origin.x + CGFloat(col) * (itemSize + spacing)
                let seatY = rect.origin.y + CGFloat(row) * (itemSize + spacing)

                let seatId = "seat_\(row)_\(col)"
                let position = CGPoint(x: seatX, y: seatY)

                // 随机设置一些座位的状态，模拟真实场景
                let randValue = (row * cols + col) % 10
                let status: UInt32
                if randValue < 6 {
                    status = 0
                } else if randValue < 8 {
                    status = 1
                } else if randValue < 9 {
                    status = 2
                } else {
                    status = 3
                }

                let seat = SeatData(seatId: seatId, status: status, selected: false, position: position)
                seats.append(seat)
            }
        }

        return seats
    }
}

extension SeatCanvasViewController: SeatCanvasViewDelegate {
    /// 是否可以选中座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - regionId: 区域ID
    ///   - seatId: 座位ID
    func seatCanvasView(_: SeatCanvasView, shouldSelectSeat _: String, seatId _: String) -> Bool {
        true
    }

    /// 选中某个座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - regionId: 区域ID
    ///   - seatId: 座位ID
    func seatCanvasView(_: SeatCanvasView, didSelectSeat regionId: String, seatId: String) {
        print(#function, regionId, seatId)
    }

    /// 取消选中某个座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - regionId: 区域ID
    ///   - seatId: 座位ID
    func seatCanvasView(_: SeatCanvasView, didDeselectSeat regionId: String, seatId: String) {
        print(#function, regionId, seatId)
    }
}
