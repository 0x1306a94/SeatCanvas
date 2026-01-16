//
//  BaseMapParserFactory.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "BaseMapParserFactory.hpp"

#include "SVGBaseMapParser.hpp"

#include <mutex>

namespace kk::parser {

std::unique_ptr<IBaseMapParser> BaseMapParserFactory::createParser(BaseMapFormat format) {
    switch (format) {
        case BaseMapFormat::SVG:
            return std::make_unique<SVGBaseMapParser>();
        case BaseMapFormat::JSON:
        case BaseMapFormat::GeoJSON:
        case BaseMapFormat::Unknown:
        default:
            return nullptr;
    }
}

std::unique_ptr<IBaseMapParser> BaseMapParserFactory::createParser(const std::string &formatName) {
    auto format = parseFormatName(formatName);
    return createParser(format);
}

std::unique_ptr<BaseMapParseResult> BaseMapParserFactory::parse(std::shared_ptr<tgfx::Data> data, BaseMapFormat format) {
    auto parser = createParser(format);
    if (!parser) {
        return nullptr;
    }
    return parser->parse(data);
}

std::unique_ptr<BaseMapParseResult> BaseMapParserFactory::parse(std::shared_ptr<tgfx::Data> data, const std::string &formatName) {
    auto format = parseFormatName(formatName);
    return parse(data, format);
}

}  // namespace kk::parser
