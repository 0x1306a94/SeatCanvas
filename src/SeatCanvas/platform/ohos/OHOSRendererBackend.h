//
//  OHOSRendererBackend.h
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#ifndef OHOSRendererBackend_h
#define OHOSRendererBackend_h

#include "core/renderer/RendererBackend.hpp"

#include <ace/xcomponent/native_interface_xcomponent.h>

namespace kk::renderer {
class OHOSRendererBackend : public RendererBackend {
  public:
    explicit OHOSRendererBackend(OH_NativeXComponent *component, void *nativeWindow);
    ~OHOSRendererBackend();

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

#endif  // OHOSRendererBackend_h
