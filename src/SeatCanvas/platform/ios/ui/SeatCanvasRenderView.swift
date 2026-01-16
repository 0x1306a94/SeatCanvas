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
            guard old != self.bounds.size else {
                return
            }
            self.updateSize()
        }
    }

    override var frame: CGRect {
        didSet {
            let old = oldValue.size
            guard old != self.frame.size else {
                return
            }
            self.updateSize()
        }
    }

    override var contentScaleFactor: CGFloat {
        didSet {
            guard oldValue != contentScaleFactor else {
                return
            }
            self.updateSize()
        }
    }

    func updateSize() {
        self.didUpdateSize?(self)
    }

    deinit {
        #if DEBUG
        print("\(type(of: self)) deinit")
        #endif
    }
}
