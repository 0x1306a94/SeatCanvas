//
//  CustomLayerType.hpp
//  SeatCanvasKit
//
//  Created by king on 2025/12/10.
//

#ifndef CustomLayerType_hpp
#define CustomLayerType_hpp

#include <tgfx/layers/LayerType.h>

namespace kk::layer {
/// 自定义 Layer 类型
enum class CustomLayerType {
    BaseMapRoot = static_cast<int>(tgfx::LayerType::Solid) + 100,  ///< 底图根节点
    Region = static_cast<int>(tgfx::LayerType::Solid) + 101,       ///< 区域
    RegionName = static_cast<int>(tgfx::LayerType::Solid) + 102,   ///< 区域名称
    SeatRoot = static_cast<int>(tgfx::LayerType::Solid) + 200,     ///< 座位根容器
    SeatAtlas = static_cast<int>(tgfx::LayerType::Solid) + 201,    ///< 座位区域容器
    Seat = static_cast<int>(tgfx::LayerType::Solid) + 202,         ///< 座位
};
};  // namespace kk::layer

#endif /* CustomLayerType_hpp */
