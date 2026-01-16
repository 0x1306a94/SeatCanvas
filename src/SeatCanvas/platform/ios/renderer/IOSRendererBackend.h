//
//  IOSRendererBackend.h
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#import "core/renderer/RendererBackend.hpp"

#import <QuartzCore/CAEAGLLayer.h>

namespace kk::renderer {
class IOSRendererBackend : public RendererBackend {
  public:
    explicit IOSRendererBackend(CAEAGLLayer *eagLayer);
    virtual ~IOSRendererBackend();

    virtual std::shared_ptr<tgfx::Window> getWindow() override;
    virtual void invalidSize() override;
    virtual tgfx::ISize getSize() override;
    virtual float getDensity() override;

  private:
    std::shared_ptr<tgfx::Window> _window{nullptr};
    CAEAGLLayer *_eagLayer;
};

};  // namespace kk::renderer
