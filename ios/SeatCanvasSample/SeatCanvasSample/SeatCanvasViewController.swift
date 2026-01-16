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

        loadBaseMap()
    }

    func loadBaseMap() {
        guard let basemapName else { return }
        guard let svgPath = baseMapBundle.path(forResource: basemapName, ofType: "svg") else {
            return
        }

        guard let data = try? Data(contentsOf: URL(fileURLWithPath: svgPath)) else {
            return
        }

        seatCanvasView.loadBaseMap(data)
    }

    @IBAction func handleTiled(_ sender: UISwitch) {
        seatCanvasView.enableTiled = sender.isOn
    }

    @IBAction func handleZoomBlur(_ sender: UISwitch) {
        seatCanvasView.enableZoomBlur = sender.isOn
    }
}

extension SeatCanvasViewController: SeatCanvasViewDelegate {
    /// 是否可以选中座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - seatId: 座位ID
    func seatCanvasView(_: SeatCanvasView, shouldSelectSeat _: String) -> Bool {
        true
    }

    /// 选中某个座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - seatId: 座位ID
    func seatCanvasView(_: SeatCanvasView, didSelectSeat seatId: String) {
        print(#function, seatId)
    }

    /// 取消选中某个座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - seatId: 座位ID
    func seatCanvasView(_: SeatCanvasView, didDeselectSeat seatId: String) {
        print(#function, seatId)
    }
}
