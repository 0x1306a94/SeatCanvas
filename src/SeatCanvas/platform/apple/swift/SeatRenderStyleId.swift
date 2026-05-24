//
//  SeatRenderStyleId.swift
//  SeatCanvas
//

import Foundation

internal import SeatCanvas_Private

@objc(KKSeatRenderStyleId)
public final class SeatRenderStyleId: NSObject {
    @objc override private init() {
        super.init()
    }

    /// 生成与 C++ 渲染器相同格式的座位样式 ID
    /// - Parameters:
    ///   - pricecode: 价档 code；nil 或空字符串表示无价档槽位
    ///   - status: 业务自定义座位状态
    ///   - selected: 是否选中
    @objc
    public class func compose(pricecode: String?, status: UInt32, selected: Bool) -> String {
        kk.bridge.SeatCanvasComposeSeatStyleId(pricecode ?? "", status, selected)
    }
}
