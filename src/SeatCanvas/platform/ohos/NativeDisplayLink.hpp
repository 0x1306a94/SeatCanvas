//
//  NativeDisplayLink.hpp
//  SeatCanvas
//
//  Created by KK on 2025/11/24.
//

#ifndef NativeDisplayLink_hpp
#define NativeDisplayLink_hpp

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

#include <napi/native_api.h>
#include <native_display_soloist/native_display_soloist.h>

#include "core/utils/DisplayLink.hpp"

namespace kk {
class NativeDisplayLink : public std::enable_shared_from_this<NativeDisplayLink>, public DisplayLink {
  public:
    static bool Init(napi_env env, napi_value exports);

    explicit NativeDisplayLink(std::function<void()> callback);
    ~NativeDisplayLink() override;

    void start() override;
    void stop() override;
    void update();

  private:
    static void DisplaySoloistCallback(long long timestamp, long long targetTimestamp, void *data);
    OH_DisplaySoloist *displaySoloist = nullptr;
    uint32_t id = 0;
    std::function<void()> callback = nullptr;
    std::atomic<bool> started = false;
};
};  // namespace kk

#endif /* NativeDisplayLink_hpp */
