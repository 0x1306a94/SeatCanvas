//
//  AndroidPlatformView.hpp
//  SeatCanvas
//
//  Created by KK on 2025/11/24.
//

#ifndef AndroidPlatformView_hpp
#define AndroidPlatformView_hpp

#include "core/renderer/PlatformView.hpp"

#include <android/native_window_jni.h>

namespace kk::renderer {

class AndroidPlatformView : public PlatformView {
  public:
    explicit AndroidPlatformView(ANativeWindow *nativeWindow, float density);
    virtual ~AndroidPlatformView();

    virtual std::shared_ptr<tgfx::Window> getWindow() override;
    virtual std::shared_ptr<tgfx::Surface> getSurface(tgfx::Context *context) override;
    virtual void invalidSize() override;
    virtual tgfx::ISize getSize() override;
    virtual float getDensity() override;

  private:
    std::shared_ptr<tgfx::Window> _window{nullptr};
    std::shared_ptr<tgfx::Surface> _surface{nullptr};
    ANativeWindow *_nativeWindow{nullptr};
    float _density{1.0f};
};

}  // namespace kk::renderer

#endif /* AndroidPlatformView_hpp */
