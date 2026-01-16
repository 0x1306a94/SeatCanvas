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

PACKED_STRUCT SeatVertex {
    float pos[2];
    float uv[2];
    int32_t styleIndex;
};
PACKED_END;

PACKED_STRUCT SeatIndex {
    uint16_t indices[6];
};
PACKED_END;

static_assert(sizeof(SeatVertex) == 20, "SeatVertex must be exactly 20 bytes");
static_assert(sizeof(SeatIndex) == 12, "SeatBaseVertex must be exactly 12 bytes");

constexpr std::size_t SEAT_VERTEX_STRIDE = sizeof(SeatVertex);

};  // namespace kk::renderer

#endif /* SeatInstanceVertex_hpp */
