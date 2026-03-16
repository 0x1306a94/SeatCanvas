//
//  BaseMapParserFactory.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef BaseMapParserFactory_hpp
#define BaseMapParserFactory_hpp

#include "BaseMapFormat.hpp"
#include "BaseMapParseResult.hpp"
#include "IBaseMapParser.hpp"

#include <cstddef>
#include <memory>
#include <string>

namespace tgfx {
class Data;
};

namespace kk::parser {

/// 底图解析器工厂
/// 负责根据格式类型创建对应的解析器并执行解析
class BaseMapParserFactory {
  public:
    /// 根据格式名称解析底图数据
    /// @param data 原始数据（shared_ptr）
    /// @param formatName 格式名称（如 "svg", "json", "geojson"）
    /// @return 解析结果，失败返回 nullptr
    static std::unique_ptr<BaseMapParseResult> parse(std::shared_ptr<tgfx::Data> data, const std::string &formatName);

    /// 根据格式名称和解析配置解析底图数据
    /// @param data 原始数据（shared_ptr）
    /// @param formatName 格式名称（如 "svg", "json", "geojson"）
    /// @param configData 解析配置数据
    /// @return 解析结果，失败返回 nullptr
    static std::unique_ptr<BaseMapParseResult> parse(std::shared_ptr<tgfx::Data> data, const std::string &formatName, std::shared_ptr<tgfx::Data> configData);

    /// 根据格式枚举解析底图数据
    /// @param data 原始数据（shared_ptr）
    /// @param format 格式枚举
    /// @return 解析结果，失败返回 nullptr
    static std::unique_ptr<BaseMapParseResult> parse(std::shared_ptr<tgfx::Data> data, BaseMapFormat format);

    /// 根据格式枚举和解析配置解析底图数据
    /// @param data 原始数据（shared_ptr）
    /// @param format 格式枚举
    /// @param configData 解析配置数据
    /// @return 解析结果，失败返回 nullptr
    static std::unique_ptr<BaseMapParseResult> parse(std::shared_ptr<tgfx::Data> data, BaseMapFormat format, std::shared_ptr<tgfx::Data> configData);

    /// 创建指定格式的解析器
    /// @param format 格式枚举
    /// @return 解析器实例，不支持则返回 nullptr
    static std::unique_ptr<IBaseMapParser> createParser(BaseMapFormat format);

    /// 创建指定格式名称的解析器
    /// @param formatName 格式名称
    /// @return 解析器实例，不支持则返回 nullptr
    static std::unique_ptr<IBaseMapParser> createParser(const std::string &formatName);
};

}  // namespace kk::parser

#endif /* BaseMapParserFactory_hpp */
