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
    ~OHOSPlatformView();

    std::shared_ptr<tgfx::Window> getWindow() override;
    virtual void invalidSize() override;
    tgfx::ISize getSize() override;
    float getDensity() override;

    static void UpdateDensity(float density);

  private:
    OH_NativeXComponent *component{nullptr};
    void *nativeWindow{nullptr};
    std::shared_ptr<tgfx::Window> window{nullptr};
};
};  // namespace kk::renderer

#endif  // OHOSPlatformView_h
