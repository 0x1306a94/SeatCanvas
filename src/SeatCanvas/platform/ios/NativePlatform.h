//
//  NativePlatform.hpp
//
//
//  Created by king on 2025/11/13.
//

#ifndef NativePlatform_hpp
#define NativePlatform_hpp

#include "platform/apple/ApplePlatform.h"

namespace kk {
class NativePlatform : public ApplePlatform {
  public:
    virtual ~NativePlatform() = default;
    virtual std::shared_ptr<DisplayLink> createDisplayLink(std::function<void()> callback) const override;
};
};  // namespace kk

#endif /* NativePlatform_hpp */
