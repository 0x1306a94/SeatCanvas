//
//  Uniform.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "Uniform.hpp"

namespace kk::renderer {
size_t Uniform::size() const {
    switch (_format) {
        case UniformFormat::Float:
            return sizeof(float);
        case UniformFormat::Float2:
            return 2 * sizeof(float);
        case UniformFormat::Float3:
            return 3 * sizeof(float);
        case UniformFormat::Float4:  // fall-through
        case UniformFormat::Float2x2:
            return 4 * sizeof(float);
        case UniformFormat::Float3x3:
            return 9 * sizeof(float);
        case UniformFormat::Float4x4:
            return 16 * sizeof(float);
        case UniformFormat::Int:
            return sizeof(int32_t);
        case UniformFormat::Int2:
            return 2 * sizeof(int32_t);
        case UniformFormat::Int3:
            return 3 * sizeof(int32_t);
        case UniformFormat::Int4:
            return 4 * sizeof(int32_t);
        case UniformFormat::Texture2DSampler:
        case UniformFormat::TextureExternalSampler:
        case UniformFormat::Texture2DRectSampler:
            return sizeof(int32_t);  // Samplers are represented as integers.
    }
    return 0;
}

}  // namespace kk::renderer
