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
#include <tgfx/core/Color.h>

namespace kk::renderer {

/**
 * 圆形样式配置
 */
class CircleSeatStyleConfig : public SeatStyleConfig {
  public:
    virtual ~CircleSeatStyleConfig() = default;

    static std::shared_ptr<CircleSeatStyleConfig> Make(const tgfx::Color &fillColor,
                                                       const tgfx::Color &overlayColor,
                                                       const tgfx::Color &checkmarkColor);

    const tgfx::Color &getFillColor() const {
        return fillColor;
    }

    const tgfx::Color &getOverlayColor() const {
        return overlayColor;
    }

    const tgfx::Color &getCheckmarkColor() const {
        return checkmarkColor;
    }

  protected:
    CircleSeatStyleConfig(const tgfx::Color &fillColor,
                          const tgfx::Color &overlayColor,
                          const tgfx::Color &checkmarkColor);

    bool isEqual(const SeatStyleConfig &other) const override;

  private:
    tgfx::Color fillColor = {};
    tgfx::Color overlayColor = {};
    tgfx::Color checkmarkColor = {};
};

}  // namespace kk::renderer

#endif /* CircleSeatStyleConfig_hpp */
