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
    ~AndroidPlatformView();

    std::shared_ptr<tgfx::Window> getWindow() override;
    virtual void invalidSize() override;
    tgfx::ISize getSize() override;
    float getDensity() override;

  private:
    std::shared_ptr<tgfx::Window> _window{nullptr};
    ANativeWindow *_nativeWindow{nullptr};
    float _density{1.0f};
};

}  // namespace kk::renderer

#endif /* AndroidPlatformView_hpp */
