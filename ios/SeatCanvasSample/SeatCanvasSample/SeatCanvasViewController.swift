//
//  SeatCanvasViewController.swift
//  SeatCanvasSample
//
//  Created by king on 2025/11/12.
//

import SeatCanvas
import UIKit

class SeatCanvasViewController: UIViewController {
    @IBOutlet weak var containerView: UIView!

    @IBOutlet weak var circleStyleSwitch: UISwitch!
    @IBOutlet weak var svgStyleSwitch: UISwitch!

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
