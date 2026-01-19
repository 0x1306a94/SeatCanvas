//
//  BaseMapZoneVertex.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef BaseMapZoneVertex_hpp
#define BaseMapZoneVertex_hpp

#include <cstddef>
#include <cstdint>

#include "core/Packed.h"

namespace kk::renderer {

PACKED_STRUCT BaseMapZoneVertex {
    float x;             // 位置 X      [offset: 0,  size: 4]
    float y;             // 位置 Y      [offset: 4,  size: 4]
    float coverage;      // AA 覆盖率   [offset: 8,  size: 4]
    int32_t colorIndex;  // 区域索引    [offset: 12, size: 4]
};
PACKED_END;

};  // namespace kk::renderer

#endif /* BaseMapZoneVertex_hpp */
