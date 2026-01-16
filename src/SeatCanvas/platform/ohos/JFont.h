//
//  JFont.h
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#ifndef SEATCANVASSAMPLE_JFONT_H
#define SEATCANVASSAMPLE_JFONT_H

#include <napi/native_api.h>
#include <string>

#include "core/Font.h"

namespace kk::js {
class JFont {
  public:
    static bool Init(napi_env env, napi_value exports);
    static napi_value ToJs(napi_env env, const kk::Font &font);
    static kk::Font FromJs(napi_env env, napi_value value);
    static std::string ClassName() {
        return "JFont";
    }

  private:
    static napi_value Constructor(napi_env env, napi_callback_info info);
};
};  // namespace kk::js

#endif  // SEATCANVASSAMPLE_JFONT_H
