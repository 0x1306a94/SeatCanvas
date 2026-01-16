//
//  DeviceLockGuard.hpp
//  SeatCanvas
//
//  Created by KK on 2025/11/14.
//

#ifndef DeviceLockGuard_hpp
#define DeviceLockGuard_hpp

#include <tgfx/gpu/Device.h>

namespace kk {
class DeviceLockGuard {
  public:
    explicit DeviceLockGuard(tgfx::Device *device)
        : _device(device)
        , _context(device != nullptr ? device->lockContext() : nullptr) {
    }

    ~DeviceLockGuard() {
        if (_device != nullptr)
            _device->unlock();
    }

    tgfx::Context *context() const {
        return _context;
    }

    explicit operator bool() const _NOEXCEPT {
        return _context != nullptr;
    }

  private:
    tgfx::Device *_device;
    tgfx::Context *_context;
};
};  // namespace kk

#endif /* DeviceLockGuard_hpp */
