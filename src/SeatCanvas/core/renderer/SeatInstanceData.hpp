//
//  SeatInstanceVertex.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef SeatInstanceVertex_hpp
#define SeatInstanceVertex_hpp

#include <cstddef>
#include <cstdint>

#include <tgfx/core/Color.h>

#include "core/Packed.h"

namespace kk::renderer {

PACKED_STRUCT SeatInstanceData {
    float x;
    float y;
    int32_t uvOffsetIndex;

    SeatInstanceData(float x, float y, int32_t uvOffsetIndex)
        : x(x)
        , y(y)
        , uvOffsetIndex(uvOffsetIndex) {
    }
};
PACKED_END;

};  // namespace kk::renderer

#endif /* SeatInstanceVertex_hpp */
