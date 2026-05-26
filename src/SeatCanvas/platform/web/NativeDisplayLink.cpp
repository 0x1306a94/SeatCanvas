//
//  NativeDisplayLink.cpp
//  SeatCanvas
//
//  Created by KK on 2026/5/26.
//

#include "NativeDisplayLink.hpp"

#include <emscripten/html5.h>

#include <tgfx/platform/Print.h>

namespace kk {

NativeDisplayLink::NativeDisplayLink(std::function<void()> callback)
    : _callback(std::move(callback)) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

NativeDisplayLink::~NativeDisplayLink() {
    cancelAnimationFrame();
    _started = false;
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

std::shared_ptr<DisplayLink> NativeDisplayLink::Make(std::function<void()> callback) {
    if (!callback) {
        return nullptr;
    }
    return std::shared_ptr<NativeDisplayLink>(new NativeDisplayLink(std::move(callback)));
}

void NativeDisplayLink::start() {
    if (_started) {
        return;
    }
    _started = true;
    _animationFrameId = emscripten_request_animation_frame(&OnFrame, this);
}

void NativeDisplayLink::stop() {
    _started = false;
    cancelAnimationFrame();
}

void NativeDisplayLink::cancelAnimationFrame() {
    if (_animationFrameId >= 0) {
        emscripten_cancel_animation_frame(_animationFrameId);
        _animationFrameId = -1;
    }
}

EM_BOOL NativeDisplayLink::OnFrame(double /*time*/, void *userData) {
    auto *self = static_cast<NativeDisplayLink *>(userData);
    if (self == nullptr || !self->_started) {
        return EM_FALSE;
    }
    if (self->_callback) {
        self->_callback();
    }
    if (self->_started) {
        self->_animationFrameId = emscripten_request_animation_frame(&OnFrame, self);
    }
    return EM_FALSE;
}

};  // namespace kk
