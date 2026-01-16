//
//  NativePlatform.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#ifndef NativePlatform_hpp
#define NativePlatform_hpp

#include "core/Platform.hpp"

namespace kk {
class NativePlatform : public Platform {
  public:
    static void InitJNI();

    virtual ~NativePlatform() = default;
    virtual bool registerFallbackFonts() const override;
    virtual std::shared_ptr<DisplayLink> createDisplayLink(std::function<void()> callback) const override;
};
};  // namespace kk

#endif /* NativePlatform_hpp */
