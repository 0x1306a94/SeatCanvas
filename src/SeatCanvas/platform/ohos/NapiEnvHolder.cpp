//
//  NapiEnvHolder.cpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#include "NapiEnvHolder.hpp"

namespace kk::js {

thread_local napi_env NapiEnvHolder::_env = nullptr;

void NapiEnvHolder::setEnv(napi_env env) {
    _env = env;
}

napi_env NapiEnvHolder::getEnv() {
    return _env;
}

}  // namespace kk::js
