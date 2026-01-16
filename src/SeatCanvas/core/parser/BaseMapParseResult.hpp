//
//  BaseMapParseResult.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef BaseMapParseResult_hpp
#define BaseMapParseResult_hpp

#include <memory>

#include <tgfx/core/Size.h>

namespace tgfx {
class Layer;
};

namespace kk::layer {
class BaseMapRootLayer;
};

namespace kk::renderer {
class BaseMapMeshBuilder;
};

namespace kk::parser {

/// 底图解析结果（统一格式）
/// 所有格式的解析器都应返回此结构
struct BaseMapParseResult {
    /// 底图原始尺寸
    tgfx::Size size;

    /// 底图网格构建器（包含所有 mesh 数据、区域信息、Path）
    std::shared_ptr<kk::renderer::BaseMapMeshBuilder> meshBuilder;

    /// 文本图层（可选，某些格式可能没有）
    std::shared_ptr<tgfx::Layer> textLayer;

    /// 小地图图层（可选，某些格式可能没有）
    std::shared_ptr<kk::layer::BaseMapRootLayer> miniLayer;

    BaseMapParseResult()
        : size(tgfx::Size::MakeEmpty())
        , meshBuilder(nullptr)
        , textLayer(nullptr)
        , miniLayer(nullptr) {
    }

    /// 检查结果是否有效
    bool isValid() const {
        return meshBuilder != nullptr && !size.isEmpty();
    }
};

}  // namespace kk::parser

#endif /* BaseMapParseResult_hpp */
