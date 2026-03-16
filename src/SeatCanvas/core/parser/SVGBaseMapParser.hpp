//
//  SVGBaseMapParser.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef SVGBaseMapParser_hpp
#define SVGBaseMapParser_hpp

#include "IBaseMapParser.hpp"
#include "core/svg/SVGParseConfig.hpp"

namespace kk::parser {

/// SVG 格式底图解析器
/// 包装现有的 SVGMeshParser 以适配统一接口
class SVGBaseMapParser : public IBaseMapParser {
  public:
    SVGBaseMapParser() = default;
    ~SVGBaseMapParser() override = default;

    /// 解析 SVG 格式的底图数据
    /// @param data SVG 二进制数据（shared_ptr）
    /// @param configData SVG 解析配置数据（shared_ptr）
    /// @return 解析结果，失败返回 nullptr
    virtual std::unique_ptr<BaseMapParseResult> parse(std::shared_ptr<tgfx::Data> data, std::shared_ptr<tgfx::Data> configData) override;

    /// 获取格式名称
    /// @return BaseMapFormat::SVG
    virtual BaseMapFormat getFormat() const override {
        return BaseMapFormat::SVG;
    }
};

}  // namespace kk::parser

#endif /* SVGBaseMapParser_hpp */
