//
//  ZoomScaleConfig.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/5.
//

#ifndef ZoomScaleConfig_hpp
#define ZoomScaleConfig_hpp

namespace kk {
struct ZoomScaleConfig {
    static constexpr float SEAT_BASE_SIZE = 36.0f;     // 座位基础尺寸(像素)
    static constexpr float ZOOM_LEVEL_SMALL = 9.0f;    // 小缩放级别
    static constexpr float ZOOM_LEVEL_MEDIUM = 18.0f;  // 中缩放级别
    static constexpr float ZOOM_LEVEL_LARGE = 30.0f;   // 大缩放级别
    static constexpr float ZOOM_LEVEL_XLARGE = 50.0f;  // 超大缩放级别
};
};  // namespace kk

#endif /* ZoomScaleConfig_hpp */
