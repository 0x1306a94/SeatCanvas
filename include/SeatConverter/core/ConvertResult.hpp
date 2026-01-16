//
//  ConvertResult.hpp
//  SeatCanvasKit
//
//  Created by king on 2025/11/19.
//

#ifndef ConvertResult_hpp
#define ConvertResult_hpp

#include <string>
#include <tuple>
#include <vector>

namespace kk::converter {
/// 转换结果
struct ConvertResult {
    bool success{false};
    std::string errorMessage;
    std::vector<std::tuple<std::string, std::string>> svgContent{};  // SVG底图内容
    std::string seatsDataJson;                                       // 座位数据JSON
};
};  // namespace kk::converter

#endif /* ConvertResult_hpp */
