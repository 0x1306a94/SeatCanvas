//
//  ZoomLevelConfig.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/5.
//

#ifndef ZoomLevelConfig_hpp
#define ZoomLevelConfig_hpp

namespace kk {
struct ZoomLevelConfig {
    /// 最近视角（座位级别）
    float seat{1.0f};
    /// 行/小块级视角
    float row{1.0f};
    /// 区域级视角
    float zone{1.0f};
    /// 全场/概览级视角（默认作为彩虹图与座位渲染切换的阈值）
    float venue{1.0f};
};
};  // namespace kk

#endif /* ZoomLevelConfig_hpp */
