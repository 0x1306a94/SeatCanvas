//
//  ApplePlatform.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/13.
//

#ifndef ApplePlatform_hpp
#define ApplePlatform_hpp

#include "core/Platform.hpp"

namespace kk {
class ApplePlatform : public Platform {
  public:
    ApplePlatform();

    virtual ~ApplePlatform() = default;
    virtual bool registerFallbackFonts() const override;
    virtual double currentMediaTime() const override;
};
};  // namespace kk

#endif /* ApplePlatform_hpp */
