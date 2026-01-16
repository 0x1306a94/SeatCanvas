//
//  SeatRegionMesh.hpp
//  SeatCanvas
//
//  Created by KK on 2025/12/21.
//

#ifndef SeatRegionMesh_hpp
#define SeatRegionMesh_hpp

#include <memory>

namespace tgfx {
class GPUBuffer;
};

namespace kk::renderer {

struct SeatRegionMesh {
    std::int32_t vertexCount = 0;
    std::int32_t indexCount = 0;
    std::shared_ptr<tgfx::GPUBuffer> vbo = {nullptr};
    std::shared_ptr<tgfx::GPUBuffer> ibo = {nullptr};

    explicit SeatRegionMesh(std::int32_t vertexCount, std::shared_ptr<tgfx::GPUBuffer> vbo, std::int32_t indexCount, std::shared_ptr<tgfx::GPUBuffer> ibo)
        : vertexCount(vertexCount)
        , vbo(std::move(vbo))
        , indexCount(indexCount)
        , ibo(std::move(ibo)) {
    }
};
};  // namespace kk::renderer

#endif /* SeatRegionMesh_hpp */
