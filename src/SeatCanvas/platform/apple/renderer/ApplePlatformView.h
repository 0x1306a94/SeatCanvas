//
//  ApplePlatformView.h
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#import "core/renderer/PlatformView.hpp"

@class MTKView;
namespace kk::renderer {
class ApplePlatformView : public PlatformView {
  public:
    explicit ApplePlatformView(MTKView *metalView);
    virtual ~ApplePlatformView();

    virtual std::shared_ptr<tgfx::Window> getWindow() override;
    virtual std::shared_ptr<tgfx::Surface> getSurface(tgfx::Context *context) override;
    virtual void invalidSize() override;
    virtual tgfx::ISize getSize() override;
    virtual float getDensity() override;
    virtual void *nativeHandle() override {
        return (__bridge void *)_metalView;
    }

  private:
    std::shared_ptr<tgfx::Window> _window{nullptr};
    std::shared_ptr<tgfx::Surface> _surface{nullptr};
    MTKView *_metalView;
};

};  // namespace kk::renderer
