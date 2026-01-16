//
//  RenderFrameMetrics.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/17.
//

#include "RenderFrameMetrics.hpp"

#include "core/Platform.hpp"

namespace kk::renderer {

float RenderFrameMetrics::currentFPS() const {
    return _cachedFPS;
}

int64_t RenderFrameMetrics::lastDrawTime() const {
    return _drawTimes.empty() ? 0 : _drawTimes.back();
}

int64_t RenderFrameMetrics::averageDrawTime() const {
    if (_drawTimes.empty()) {
        return 0;
    }
    int64_t total = 0;
    for (const auto &drawTime : _drawTimes) {
        total += drawTime;
    }
    return total / static_cast<int64_t>(_drawTimes.size());
}

bool RenderFrameMetrics::isFirstFrame() const {
    return _frameTimeStamps.empty();
}

void RenderFrameMetrics::recordFrame(int64_t drawTime) {
    auto currentTime = static_cast<int64_t>(Platform::Current()->currentMediaTime());

    _frameTimeStamps.push_back(currentTime);
    _drawTimes.push_back(drawTime);

    pruneOldFrames(currentTime);

    if (_frameTimeStamps.size() >= 2) {
        auto duration = _frameTimeStamps.back() - _frameTimeStamps.front();
        if (duration >= 1000) {
            _cachedFPS = static_cast<float>((_frameTimeStamps.size() - 1) * 1000) / static_cast<float>(duration);
        }
    }
}

void RenderFrameMetrics::resetFrames() {
    _frameTimeStamps.clear();
    _drawTimes.clear();
    _cachedFPS = 0.0f;
}

void RenderFrameMetrics::pruneOldFrames(int64_t currentTime) {
    auto cutoffTime = currentTime - kWindowMs;

    while (!_frameTimeStamps.empty() && _frameTimeStamps.front() < cutoffTime) {
        _frameTimeStamps.pop_front();
        if (!_drawTimes.empty()) {
            _drawTimes.pop_front();
        }
    }
}

};  // namespace kk::renderer
