//
//  Platform.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/13.
//

#ifndef Platform_hpp
#define Platform_hpp

#include <functional>
#include <memory>
#include <vector>

namespace kk {
class DisplayLink;
class Platform {
  public:
    static const Platform *Current();

    virtual ~Platform() = default;

    virtual bool registerFallbackFonts() const;

    /// 当前媒体时间（毫秒）
    virtual double currentMediaTime() const;

    virtual std::shared_ptr<DisplayLink> createDisplayLink(std::function<void()> callback, void *userInfo) const;
};
};  // namespace kk

#endif /* Platform_hpp */
