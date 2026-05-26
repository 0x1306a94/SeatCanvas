//
//  WebPlatformView.hpp
//  SeatCanvas
//
//  Created by king on 2026/5/26.
//

#ifndef WebPlatformView_hpp
#define WebPlatformView_hpp

#include <memory>
#include <string>

#include "core/renderer/PlatformView.hpp"

namespace tgfx {
class Window;
class Surface;
class Context;
};  // namespace tgfx

namespace kk::renderer {
class WebPlatformView : public PlatformView {
  public:
    explicit WebPlatformView(const std::string &canvasID);
    ~WebPlatformView() override;

    std::shared_ptr<tgfx::Window> getWindow() override;
    std::shared_ptr<tgfx::Surface> getSurface(tgfx::Context *context) override;
    void invalidSize() override;
    tgfx::ISize getSize() override;
    float getDensity() override;

  private:
    void refreshDensity();

    std::string _canvasID = {};
    std::shared_ptr<tgfx::Window> _window = {nullptr};
    std::shared_ptr<tgfx::Surface> _surface = {nullptr};
    float _density = {1.0f};
};
};  // namespace kk::renderer

#endif /* WebPlatformView_hpp */
