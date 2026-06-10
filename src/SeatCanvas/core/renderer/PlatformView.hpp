//
//  PlatformView.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#ifndef PlatformView_hpp
#define PlatformView_hpp

#include <memory>

#include <tgfx/core/Size.h>

namespace tgfx {
class Window;
class Surface;
class Context;
class Device;
};  // namespace tgfx

namespace kk::renderer {
class PlatformView {
  public:
    virtual ~PlatformView();
    virtual std::shared_ptr<tgfx::Device> getDevice();
    virtual std::shared_ptr<tgfx::Window> getWindow() = 0;
    virtual std::shared_ptr<tgfx::Surface> getSurface(tgfx::Context *context) = 0;
    virtual void invalidSize() = 0;
    virtual tgfx::ISize getSize() = 0;
    virtual float getDensity() = 0;
    virtual void *nativeHandle();
};

};  // namespace kk::renderer

#endif /* PlatformView_hpp */
