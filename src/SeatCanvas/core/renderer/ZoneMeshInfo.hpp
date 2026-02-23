//
//  ZoneMeshInfo.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef ZoneMeshInfo_hpp
#define ZoneMeshInfo_hpp

#include "BaseMapZoneVertex.hpp"

#include <tgfx/core/Color.h>
#include <tgfx/core/Path.h>
#include <tgfx/core/Rect.h>
#include <tgfx/core/Stroke.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace kk::renderer {
struct ZoneMeshInfo {
    std::string zoneId = {""};
    std::unordered_map<std::string, std::string> attributes = {};

    tgfx::Rect fillBounds = {tgfx::Rect::MakeEmpty()};
    tgfx::Rect strokeBounds = {tgfx::Rect::MakeEmpty()};
    std::shared_ptr<tgfx::Path> path = {nullptr};
    std::optional<tgfx::Color> fillColor = {std::nullopt};
    std::optional<tgfx::Color> strokeColor = {std::nullopt};
    std::optional<tgfx::Color> alternateColor = {std::nullopt};
    std::vector<BaseMapZoneVertex> fillVertices = {};
    std::vector<BaseMapZoneVertex> strokeVertices = {};
    float strokeWidth = 1.0f;
    bool strokeOnTop = true;

    ZoneMeshInfo() = default;

    bool isValid() const {
        if (!path) {
            return false;
        }

        if (fillVertices.empty() && strokeVertices.empty()) {
            return false;
        }

        return true;
    }
};

}  // namespace kk::renderer

#endif /* ZoneMeshInfo_hpp */
