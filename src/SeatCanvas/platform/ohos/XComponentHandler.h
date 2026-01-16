//
//  XComponentHandler.h
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#ifndef XComponentHandler_h
#define XComponentHandler_h

#include <ace/xcomponent/native_interface_xcomponent.h>
#include <napi/native_api.h>
#include <native_window/external_window.h>

#include <memory>
#include <string>

namespace kk::js {
class XComponentListener {
  public:
    virtual ~XComponentListener(){};
    virtual void onSurfaceCreated(OH_NativeXComponent *component, NativeWindow *window) = 0;
    virtual void onSurfaceSizeChanged() = 0;
    virtual void onSurfaceDestroyed() = 0;
};

class XComponentHandler {
  public:
    static bool Init(napi_env env, napi_value exports);
    static bool AddListener(const std::string &xComponentID, std::weak_ptr<XComponentListener> listener);
    static bool RemoveListener(const std::string &xComponentID);
};
};  // namespace kk::js

#endif  // XComponentHandler_h
