//
//  BaseMapFormat.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef BaseMapFormat_hpp
#define BaseMapFormat_hpp

#include <string>

namespace kk::parser {

/// 底图格式类型
enum class BaseMapFormat {
    /// SVG 格式
    SVG,
    /// JSON 格式（自定义格式）
    JSON,
    /// GeoJSON 格式
    GeoJSON,
    /// 未知格式
    Unknown
};

/// 格式名称常量
namespace FormatName {
constexpr const char *SVG = "svg";
constexpr const char *JSON = "json";
constexpr const char *GeoJSON = "geojson";
}  // namespace FormatName

/// 将格式名称转换为枚举
/// @param formatName 格式名称（不区分大小写）
/// @return 格式枚举
BaseMapFormat parseFormatName(const std::string &formatName);

/// 将格式枚举转换为名称
/// @param format 格式枚举
/// @return 格式名称
std::string formatNameToString(BaseMapFormat format);

}  // namespace kk::parser

#endif /* BaseMapFormat_hpp */
