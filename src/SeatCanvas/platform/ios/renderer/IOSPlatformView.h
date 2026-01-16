//
//  IOSPlatformView.h
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#import "core/renderer/PlatformView.hpp"

#import <QuartzCore/CAEAGLLayer.h>

namespace kk::renderer {
class IOSPlatformView : public PlatformView {
  public:
    explicit IOSPlatformView(CAEAGLLayer *eagLayer);
    virtual ~IOSPlatformView();

    virtual std::shared_ptr<tgfx::Window> getWindow() override;
    virtual void invalidSize() override;
    virtual tgfx::ISize getSize() override;
    virtual float getDensity() override;

  private:
    std::shared_ptr<tgfx::Window> _window{nullptr};
    CAEAGLLayer *_eagLayer;
};

};  // namespace kk::renderer
