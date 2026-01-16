//
//  AndroidRendererBackend.hpp
//  SeatCanvas
//
//  Created by KK on 2025/11/24.
//

#ifndef AndroidRendererBackend_hpp
#define AndroidRendererBackend_hpp

#include "core/renderer/RendererBackend.hpp"

#include <android/native_window_jni.h>

namespace kk::renderer {

class AndroidRendererBackend : public RendererBackend {
  public:
    explicit AndroidRendererBackend(ANativeWindow *nativeWindow, float density);
    ~AndroidRendererBackend();

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

#endif /* AndroidRendererBackend_hpp */
