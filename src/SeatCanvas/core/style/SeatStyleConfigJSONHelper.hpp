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
#include "core/style/SeatStyleType.hpp"

#include <nlohmann/json.hpp>
#include <tgfx/platform/Print.h>

namespace nlohmann {

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
        };

        const auto &overlayColor = config->getOverlayColor();
        if (overlayColor.has_value()) {
            j["overlay"] = kk::renderer::ColorToARGBHex(*overlayColor);
        }

        const auto &checkmarkColor = config->getCheckmarkColor();
        if (checkmarkColor.has_value()) {
            j["checkmark"] = kk::renderer::ColorToARGBHex(*checkmarkColor);
        }
    }

    static void from_json(const json &j, std::shared_ptr<kk::renderer::CircleSeatStyleConfig> &config) {
        if (!j.contains("fill")) {
            tgfx::PrintError("Missing fill field in CircleSeatStyleConfig");
            config = nullptr;
            return;
        }

        std::string fillHex = j.value("fill", "");
        std::string overlayHex = j.value("overlay", "");
        std::string checkmarkHex = j.value("checkmark", "");

        tgfx::Color fillColor = {};
        if (!kk::renderer::ParseColorFromARGBHex(fillHex, fillColor)) {
            tgfx::PrintError("Failed to parse fill color hex string");
            config = nullptr;
            return;
        }

        std::optional<tgfx::Color> overlayColor = std::nullopt;
        if (!overlayHex.empty()) {
            tgfx::Color parsedOverlayColor = {};
            if (!kk::renderer::ParseColorFromARGBHex(overlayHex, parsedOverlayColor)) {
                tgfx::PrintError("Failed to parse overlay color hex string");
                config = nullptr;
                return;
            }
            overlayColor = parsedOverlayColor;
        }

        std::optional<tgfx::Color> checkmarkColor = std::nullopt;
        if (!checkmarkHex.empty()) {
            tgfx::Color parsedCheckmarkColor = {};
            if (!kk::renderer::ParseColorFromARGBHex(checkmarkHex, parsedCheckmarkColor)) {
                tgfx::PrintError("Failed to parse checkmark color hex string");
                config = nullptr;
                return;
            }
            checkmarkColor = parsedCheckmarkColor;
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
                std::shared_ptr<kk::renderer::CircleSeatStyleConfig> circleConfig = nullptr;
                j.get_to(circleConfig);
                config = circleConfig;
                break;
            }
            case kk::renderer::SeatStyleType::SVG: {
                std::shared_ptr<kk::renderer::SVGSeatStyleConfig> svgConfig = nullptr;
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
