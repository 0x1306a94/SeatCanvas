//
//  SeatCanvasRenderView.swift
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

import UIKit

class SeatCanvasRenderView: UIView {
    var didUpdateSize: ((SeatCanvasRenderView) -> Void)?
    override class var layerClass: AnyClass {
        CAEAGLLayer.classForCoder()
    }

//    override func layoutSubviews() {
//        super.layoutSubviews()
//        self.updateSize()
//    }

    override var bounds: CGRect {
        didSet {
            let old = oldValue.size
            guard old != bounds.size else {
                return
            }
            updateSize()
        }
    }

    override var frame: CGRect {
        didSet {
            let old = oldValue.size
            guard old != frame.size else {
                return
            }
            updateSize()
        }
    }

    override var contentScaleFactor: CGFloat {
        didSet {
            guard oldValue != contentScaleFactor else {
                return
            }
            updateSize()
        }
    }

    func updateSize() {
        didUpdateSize?(self)
    }

    deinit {
        #if DEBUG
            print("\(type(of: self)) deinit")
        #endif
    }
}
