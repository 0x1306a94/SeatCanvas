//
//  RendererBackend.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#ifndef RendererBackend_hpp
#define RendererBackend_hpp

#include <memory>

#include <tgfx/core/Size.h>

namespace tgfx {
class Window;
};

namespace kk::renderer {
class RendererBackend {
  public:
    virtual ~RendererBackend() = default;
    virtual std::shared_ptr<tgfx::Window> getWindow() = 0;
    virtual void invalidSize() = 0;
    virtual tgfx::ISize getSize() = 0;
    virtual float getDensity() = 0;
};

};  // namespace kk::renderer

#endif /* RendererBackend_hpp */
