//
//  NapiEnvHolder.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#ifndef NapiEnvHolder_hpp
#define NapiEnvHolder_hpp

#include <mutex>
#include <napi/native_api.h>
#include <thread>
#include <unordered_map>

namespace kk::js {

class NapiEnvHolder {
  public:
    static void setEnv(napi_env env);
    static napi_env getEnv();

  private:
    static thread_local napi_env _env;
};

}  // namespace kk::js

#endif /* NapiEnvHolder_hpp */
