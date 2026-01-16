//
//  BaseMapRegionVertex.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef BaseMapRegionVertex_hpp
#define BaseMapRegionVertex_hpp

#include <cstddef>
#include <cstdint>

#include "core/Packed.h"

namespace kk::renderer {

/**
 * 区域底图填充顶点结构 (C-style POD)
 * 严格 1 字节对齐，总大小 16 bytes
 */
PACKED_STRUCT BaseMapRegionVertex {
    float x;             // 位置 X      [offset: 0,  size: 4]
    float y;             // 位置 Y      [offset: 4,  size: 4]
    float coverage;      // AA 覆盖率   [offset: 8,  size: 4]
    int32_t colorIndex;  // 区域索引    [offset: 12, size: 4]
};
PACKED_END;

};  // namespace kk::renderer

#endif /* BaseMapRegionVertex_hpp */
