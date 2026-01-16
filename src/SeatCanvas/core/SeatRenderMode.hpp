//
//  SeatRenderMode.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/5.
//

#ifndef SeatRenderMode_hpp
#define SeatRenderMode_hpp

namespace kk {
/// 座位渲染模式
enum class SeatRenderMode {
    /// 点击区域模式：先渲染区域，点击某个区域后进入渲染这个区域的座位
    /// 座位的坐标是相对于区域内的
    ClickToEnter,
    /// 缩放级别模式：渲染所有区域，当放大到一定级别时渲染座位
    /// 座位的坐标是相对于整个画布的
    ZoomBased,
};
};  // namespace kk

#endif /* SeatRenderMode_hpp */
