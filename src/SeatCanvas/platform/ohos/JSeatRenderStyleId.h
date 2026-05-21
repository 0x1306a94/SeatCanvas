//
//  JSeatRenderStyleId.h
//  SeatCanvas
//

#ifndef JSeatRenderStyleId_h
#define JSeatRenderStyleId_h

#include <napi/native_api.h>

namespace kk::js {

class JSeatRenderStyleId {
  public:
    static bool Init(napi_env env, napi_value exports);

  private:
    static napi_value Constructor(napi_env env, napi_callback_info info);
};

};  // namespace kk::js

#endif /* JSeatRenderStyleId_h */
