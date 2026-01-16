//
//  SVGSeatStyleConfig.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#ifndef SVGSeatStyleConfig_hpp
#define SVGSeatStyleConfig_hpp

#include "SeatStyleConfig.hpp"
#include "SeatStyleType.hpp"

#include <string>

namespace kk::renderer {

class SVGSeatStyleConfig : public SeatStyleConfig {
  public:
    virtual ~SVGSeatStyleConfig() = default;

    static std::shared_ptr<SVGSeatStyleConfig> Make(const std::string &content);

    const std::string &getContent() const {
        return content;
    }

  protected:
    SVGSeatStyleConfig(const std::string &content);

    bool isEqual(const SeatStyleConfig &other) const override;

  private:
    std::string content = {};
};
};  // namespace kk::renderer

#endif /* SVGSeatStyleConfig_hpp */
