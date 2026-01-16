//
//  RegionInfo.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/14.
//

#ifndef RegionInfo_hpp
#define RegionInfo_hpp

#include <string>
#include <unordered_map>

#include <tgfx/core/Rect.h>

namespace kk {

/// 区域信息
struct RegionInfo {
    tgfx::Rect bounds;
    std::string regionId;
    std::unordered_map<std::string, std::string> attributes;

    RegionInfo() = default;
    RegionInfo(const tgfx::Rect &bounds, const std::string &regionId)
        : bounds(bounds)
        , regionId(regionId) {
    }
};

};  // namespace kk

#endif /* RegionInfo_hpp */
