//
//  IBaseMapParser.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef IBaseMapParser_hpp
#define IBaseMapParser_hpp

#include "BaseMapFormat.hpp"
#include "BaseMapParseResult.hpp"

#include <memory>

namespace tgfx {
class Data;
};

namespace kk::parser {

/// 底图解析器接口
/// 所有格式的解析器都应实现此接口
class IBaseMapParser {
  public:
    virtual ~IBaseMapParser() = default;

    /// 解析底图数据
    /// @param data 原始数据（shared_ptr）
    /// @return 解析结果，失败返回 nullptr
    virtual std::unique_ptr<BaseMapParseResult> parse(std::shared_ptr<tgfx::Data> data) = 0;

    /// 获取支持的格式
    /// @return 格式
    virtual BaseMapFormat getFormat() const = 0;
};

}  // namespace kk::parser

#endif /* IBaseMapParser_hpp */
