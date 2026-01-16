//
//  NativeDisplayLink.hpp
//  SeatCanvas
//
//  Created by KK on 2025/11/24.
//

#ifndef NativeDisplayLink_hpp
#define NativeDisplayLink_hpp

#include <functional>
#include <mutex>

#include "core/utils/DisplayLink.hpp"

#include "jni/JNIHelper.hpp"

namespace kk {
class NativeDisplayLink : public DisplayLink {
  public:
    static void InitJNI(JNIEnv *env);
    static std::shared_ptr<DisplayLink> Make(std::function<void()> callback);
    ~NativeDisplayLink() override;

    void start() override;
    void stop() override;
    void update();

  private:
    explicit NativeDisplayLink(std::function<void()> callback);

    kk::jni::Global<jobject> _animator = nullptr;
    std::function<void()> _callback = nullptr;
    std::atomic<bool> _started = false;
};
};  // namespace kk

#endif /* NativeDisplayLink_hpp */
