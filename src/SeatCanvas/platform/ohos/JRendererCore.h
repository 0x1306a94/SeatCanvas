//
//  JRendererCore.h
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#ifndef SEATCANVASSAMPLE_JRENDERERCORE_H
#define SEATCANVASSAMPLE_JRENDERERCORE_H

#include <napi/native_api.h>

#include <memory>
#include <mutex>

#include "XComponentHandler.h"

#include <tgfx/core/Color.h>

namespace kk::renderer {
class SeatCanvasCoreRenderer;
};

namespace kk::js {
class OHOSSeatCanvasCoreRendererDelegate;
class JRendererCore : public XComponentListener {
  public:
    std::string id;

    static bool Init(napi_env env, napi_value exports);
    static inline std::string ClassName() {
        return "JRendererCore";
    }

    explicit JRendererCore(const std::string &id);

    virtual ~JRendererCore();

    void onSurfaceCreated(OH_NativeXComponent *component, NativeWindow *window) override;
    void onSurfaceDestroyed() override;
    void onSurfaceSizeChanged() override;

    void start();
    void stop();

    void setBackgroundColor(const tgfx::Color &color);

    kk::renderer::SeatCanvasCoreRenderer *internalRenderer() {
        return renderer.get();
    }

    std::shared_ptr<OHOSSeatCanvasCoreRendererDelegate> getDelegate() {
        return delegate;
    }

  private:
    static napi_value Constructor(napi_env env, napi_callback_info info);
    std::unique_ptr<kk::renderer::SeatCanvasCoreRenderer> renderer = {nullptr};
    std::shared_ptr<OHOSSeatCanvasCoreRendererDelegate> delegate = {nullptr};

    std::mutex locker;
};

};  // namespace kk::js

#endif  // SEATCANVASSAMPLE_JRENDERERCORE_H
