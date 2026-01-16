//
//  BaseMapFormat.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "BaseMapFormat.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace kk::parser {

BaseMapFormat parseFormatName(const std::string &formatName) {
    if (formatName.empty()) {
        return BaseMapFormat::Unknown;
    }

    std::string lowerName = formatName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    if (lowerName == FormatName::SVG || lowerName == ".svg") {
        return BaseMapFormat::SVG;
    } else if (lowerName == FormatName::JSON || lowerName == ".json") {
        return BaseMapFormat::JSON;
    } else if (lowerName == FormatName::GeoJSON || lowerName == ".geojson") {
        return BaseMapFormat::GeoJSON;
    }

    return BaseMapFormat::Unknown;
}

std::string formatNameToString(BaseMapFormat format) {
    switch (format) {
        case BaseMapFormat::SVG:
            return FormatName::SVG;
        case BaseMapFormat::JSON:
            return FormatName::JSON;
        case BaseMapFormat::GeoJSON:
            return FormatName::GeoJSON;
        case BaseMapFormat::Unknown:
        default:
            return "unknown";
    }
}

}  // namespace kk::parser
