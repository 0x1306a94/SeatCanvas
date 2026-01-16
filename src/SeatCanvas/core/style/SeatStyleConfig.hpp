//
//  SeatStyleConfig.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef SeatStyleConfig_hpp
#define SeatStyleConfig_hpp

#include "SeatStyleType.hpp"

#include <memory>

namespace kk::renderer {

/**
 * 座位样式配置基类
 * 业务层可以通过配置来定制样式的视觉效果
 * 每种样式类型都有对应的配置子类
 */
class SeatStyleConfig {
  public:
    virtual ~SeatStyleConfig() = default;

    SeatStyleType getType() const {
        return _type;
    }

    bool operator==(const SeatStyleConfig &other) const {
        if (_type != other._type) {
            return false;
        }
        return isEqual(other);
    }

  protected:
    explicit SeatStyleConfig(SeatStyleType type)
        : _type(type) {
    }

    virtual bool isEqual(const SeatStyleConfig &other) const = 0;

  private:
    SeatStyleType _type;
};

}  // namespace kk::renderer

#endif /* SeatStyleConfig_hpp */
