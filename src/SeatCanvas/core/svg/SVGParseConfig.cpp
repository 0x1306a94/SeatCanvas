#include "SVGParseConfig.hpp"

#include "core/Log.hpp"
#include <nlohmann/json.hpp>
#include <tgfx/core/Data.h>

namespace kk::svg {

static std::vector<std::string> ParseZoneIdAttributeNames(const nlohmann::json &json) {
    std::vector<std::string> zoneIdAttributeNames = {"zoneId"};
    if (!json.contains("zoneIdAttributeNames")) {
        return zoneIdAttributeNames;
    }

    const auto &zoneIdAttributeNamesJSON = json["zoneIdAttributeNames"];
    if (!zoneIdAttributeNamesJSON.is_array()) {
        SC_LOG_ERROR("Invalid SVG parse config JSON: 'zoneIdAttributeNames' is not an array");
        return zoneIdAttributeNames;
    }

    std::vector<std::string> parsedZoneIdAttributeNames = {};
    parsedZoneIdAttributeNames.reserve(zoneIdAttributeNamesJSON.size());
    for (const auto &item : zoneIdAttributeNamesJSON) {
        if (!item.is_string()) {
            SC_LOG_ERROR("Invalid SVG parse config JSON: 'zoneIdAttributeNames' contains non-string values");
            return zoneIdAttributeNames;
        }
        auto attributeName = item.get<std::string>();
        if (attributeName.empty()) {
            SC_LOG_ERROR("Invalid SVG parse config JSON: 'zoneIdAttributeNames' contains empty values");
            return zoneIdAttributeNames;
        }
        parsedZoneIdAttributeNames.push_back(attributeName);
    }

    if (parsedZoneIdAttributeNames.empty()) {
        SC_LOG_ERROR("Invalid SVG parse config JSON: 'zoneIdAttributeNames' is empty");
        return zoneIdAttributeNames;
    }
    return parsedZoneIdAttributeNames;
}

SVGParseConfig SVGParseConfig::FromJSON(const void *bytes, size_t len) {
    SVGParseConfig config;
    if (bytes == nullptr || len == 0) {
        return config;
    }

    std::string jsonString(reinterpret_cast<const char *>(bytes), len);
    if (!nlohmann::json::accept(jsonString)) {
        SC_LOG_ERROR("Invalid SVG parse config JSON format");
        return config;
    }

    auto json = nlohmann::json::parse(jsonString, nullptr, false);
    if (json.is_discarded()) {
        SC_LOG_ERROR("Failed to parse SVG parse config JSON");
        return config;
    }

    if (!json.is_object()) {
        SC_LOG_ERROR("Invalid SVG parse config JSON: expected object");
        return config;
    }

    config.zoneIdAttributeNames = ParseZoneIdAttributeNames(json);

    if (json.contains("zoneIdAttributeName")) {
        const auto zoneIdAttributeName = json.value("zoneIdAttributeName", std::string(""));
        if (zoneIdAttributeName.empty()) {
            SC_LOG_ERROR("Invalid SVG parse config JSON: 'zoneIdAttributeName' is empty");
            return config;
        }
        config.zoneIdAttributeNames = {zoneIdAttributeName};
    }
    return config;
}

SVGParseConfig SVGParseConfig::FromJSON(std::shared_ptr<tgfx::Data> data) {
    if (data == nullptr || data->empty()) {
        return {};
    }
    return FromJSON(data->data(), data->size());
}

}  // namespace kk::svg
