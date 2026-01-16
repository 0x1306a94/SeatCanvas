//
//  ColorHexParser.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef ColorHexParser_hpp
#define ColorHexParser_hpp

#include <string>
#include <tgfx/core/Color.h>

namespace kk::renderer {

/**
 * 从十六进制颜色字符串解析颜色
 * 支持格式：#AARRGGBB 或 #RRGGBB（默认 alpha = 255）
 * @param hexString 十六进制颜色字符串，如 "#EB484AFF"
 * @param outColor 输出颜色对象
 * @return 是否解析成功
 */
bool ParseColorFromARGBHex(const std::string &hexString, tgfx::Color &outColor);
std::string ColorToARGBHex(const tgfx::Color &color);

}  // namespace kk::renderer

#endif /* ColorHexParser_hpp */
