//
//  SeatStyleConfigJSONHelper.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/15.
//

#ifndef SeatStyleConfigJSONHelper_hpp
#define SeatStyleConfigJSONHelper_hpp

#include "core/style/CircleSeatStyleConfig.hpp"
#include "core/style/ColorHexParser.hpp"
#include "core/style/SVGSeatStyleConfig.hpp"
#include "core/style/SeatStyleConfig.hpp"
#include "core/style/SeatStyleKey.hpp"
#include "core/style/SeatStyleType.hpp"

#include <nlohmann/json.hpp>
#include <tgfx/platform/Print.h>

namespace nlohmann {

template <>
struct adl_serializer<kk::SeatStyleKey> {
    static void to_json(json &j, const kk::SeatStyleKey &key) {
        j = json{
            {"status", key.getStatus()},
            {"selected", key.isSelected()},
        };
    }

    static void from_json(const json &j, kk::SeatStyleKey &key) {
        if (!j.contains("status") || !j.contains("selected")) {
            tgfx::PrintError("SeatStyleKey: missing required fields");
            key = kk::SeatStyleKey(0, false);
            return;
        }
        uint32_t status = j.value("status", 0U);
        bool selected = j.value("selected", false);
        key = kk::SeatStyleKey(status, selected);
    }
};

template <>
struct adl_serializer<std::shared_ptr<kk::renderer::CircleSeatStyleConfig>> {
    static void to_json(json &j, const std::shared_ptr<kk::renderer::CircleSeatStyleConfig> &config) {
        if (!config) {
            j = json::object();
            return;
        }

        j = json{
            {"type", static_cast<int>(config->getType())},
            {"fill", kk::renderer::ColorToARGBHex(config->getFillColor())},
            {"overlay", kk::renderer::ColorToARGBHex(config->getOverlayColor())},
            {"checkmark", kk::renderer::ColorToARGBHex(config->getCheckmarkColor())},
        };
    }

    static void from_json(const json &j, std::shared_ptr<kk::renderer::CircleSeatStyleConfig> &config) {
        if (!j.contains("fill") || !j.contains("overlay") || !j.contains("checkmark")) {
            tgfx::PrintError("Missing color fields in CircleSeatStyleConfig");
            config = nullptr;
            return;
        }

        std::string fillHex = j.value("fill", "");
        std::string overlayHex = j.value("overlay", "");
        std::string checkmarkHex = j.value("checkmark", "");

        tgfx::Color fillColor, overlayColor, checkmarkColor;
        if (!kk::renderer::ParseColorFromARGBHex(fillHex, fillColor) ||
            !kk::renderer::ParseColorFromARGBHex(overlayHex, overlayColor) ||
            !kk::renderer::ParseColorFromARGBHex(checkmarkHex, checkmarkColor)) {
            tgfx::PrintError("Failed to parse color hex strings");
            config = nullptr;
            return;
        }

        config = kk::renderer::CircleSeatStyleConfig::Make(fillColor, overlayColor, checkmarkColor);
    }
};

template <>
struct adl_serializer<std::shared_ptr<kk::renderer::SVGSeatStyleConfig>> {
    static void to_json(json &j, const std::shared_ptr<kk::renderer::SVGSeatStyleConfig> &config) {
        if (!config) {
            j = json::object();
            return;
        }

        j = json{
            {"type", static_cast<int>(config->getType())},
            {"content", config->getContent()},
        };
    }

    static void from_json(const json &j, std::shared_ptr<kk::renderer::SVGSeatStyleConfig> &config) {
        if (!j.contains("content")) {
            tgfx::PrintError("Missing content fields in SVGSeatStyleConfig");
            config = nullptr;
            return;
        }

        auto content = j.value("content", "");
        if (content.empty()) {
            tgfx::PrintError("Missing content fields in SVGSeatStyleConfig");
            config = nullptr;
            return;
        }

        config = kk::renderer::SVGSeatStyleConfig::Make(content);
    }
};

template <>
struct adl_serializer<std::shared_ptr<kk::renderer::SeatStyleConfig>> {
    static void to_json(json &j, const std::shared_ptr<kk::renderer::SeatStyleConfig> &config) {
        if (!config) {
            j = json::object();
            return;
        }

        switch (config->getType()) {
            case kk::renderer::SeatStyleType::Circle: {
                auto circleConfig = std::static_pointer_cast<kk::renderer::CircleSeatStyleConfig>(config);
                j = circleConfig;
                break;
            }
            case kk::renderer::SeatStyleType::SVG: {
                auto svgConfig = std::static_pointer_cast<kk::renderer::SVGSeatStyleConfig>(config);
                j = svgConfig;
                break;
            }
            default:
                tgfx::PrintError("Unsupported SeatStyleType");
                j = json::object();
                break;
        }
    }

    static void from_json(const json &j, std::shared_ptr<kk::renderer::SeatStyleConfig> &config) {
        if (!j.contains("type")) {
            tgfx::PrintError("Missing 'type' field in config");
            config = nullptr;
            return;
        }

        int typeValue = j.value("type", -1);
        if (typeValue < 0) {
            tgfx::PrintError("Invalid 'type' field in config");
            config = nullptr;
            return;
        }

        kk::renderer::SeatStyleType type = static_cast<kk::renderer::SeatStyleType>(typeValue);
        switch (type) {
            case kk::renderer::SeatStyleType::Circle: {
                std::shared_ptr<kk::renderer::CircleSeatStyleConfig> circleConfig;
                j.get_to(circleConfig);
                config = circleConfig;
                break;
            }
            case kk::renderer::SeatStyleType::SVG: {
                std::shared_ptr<kk::renderer::SVGSeatStyleConfig> svgConfig;
                j.get_to(svgConfig);
                config = svgConfig;
                break;
            }
            default:
                tgfx::PrintError("Unsupported SeatStyleType: %d", typeValue);
                config = nullptr;
                break;
        }
    }
};

};  // namespace nlohmann

#endif /* SeatStyleConfigJSONHelper_hpp */
