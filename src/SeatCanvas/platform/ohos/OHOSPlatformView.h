//
//  OHOSPlatformView.h
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#ifndef OHOSPlatformView_h
#define OHOSPlatformView_h

#include "core/renderer/PlatformView.hpp"

#include <ace/xcomponent/native_interface_xcomponent.h>

namespace kk::renderer {
class OHOSPlatformView : public PlatformView {
  public:
    explicit OHOSPlatformView(OH_NativeXComponent *component, void *nativeWindow);
    virtual ~OHOSPlatformView();

    virtual std::shared_ptr<tgfx::Window> getWindow() override;
    virtual std::shared_ptr<tgfx::Surface> getSurface(tgfx::Context *context) override;
    virtual void invalidSize() override;
    virtual tgfx::ISize getSize() override;
    virtual float getDensity() override;

    static void UpdateDensity(float density);

  private:
    OH_NativeXComponent *_component{nullptr};
    void *_nativeWindow{nullptr};
    std::shared_ptr<tgfx::Window> _window{nullptr};
    std::shared_ptr<tgfx::Surface> _surface{nullptr};
};
};  // namespace kk::renderer

#endif  // OHOSPlatformView_h
