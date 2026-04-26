//
//  CircleSeatStyleConfig.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef CircleSeatStyleConfig_hpp
#define CircleSeatStyleConfig_hpp

#include "SeatStyleConfig.hpp"
#include "SeatStyleType.hpp"

#include <memory>
#include <optional>
#include <tgfx/core/Color.h>

namespace kk::renderer {

/**
 * 圆形样式配置
 */
class CircleSeatStyleConfig : public SeatStyleConfig {
  public:
    virtual ~CircleSeatStyleConfig() = default;

    static std::shared_ptr<CircleSeatStyleConfig> Make(const tgfx::Color &fillColor,
                                                       const std::optional<tgfx::Color> &overlayColor = std::nullopt,
                                                       const std::optional<tgfx::Color> &checkmarkColor = std::nullopt);

    const tgfx::Color &getFillColor() const {
        return fillColor;
    }

    const std::optional<tgfx::Color> &getOverlayColor() const {
        return overlayColor;
    }

    const std::optional<tgfx::Color> &getCheckmarkColor() const {
        return checkmarkColor;
    }

  protected:
    CircleSeatStyleConfig(const tgfx::Color &fillColor,
                          const std::optional<tgfx::Color> &overlayColor,
                          const std::optional<tgfx::Color> &checkmarkColor);

    bool isEqual(const SeatStyleConfig &other) const override;

  private:
    tgfx::Color fillColor = {};
    std::optional<tgfx::Color> overlayColor = std::nullopt;
    std::optional<tgfx::Color> checkmarkColor = std::nullopt;
};

}  // namespace kk::renderer

#endif /* CircleSeatStyleConfig_hpp */
