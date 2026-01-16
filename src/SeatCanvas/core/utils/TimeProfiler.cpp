//
//  TimeProfiler.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/15.
//

#include "TimeProfiler.hpp"

#if ENABLE_TIME_PROFILER

#include <tgfx/platform/Print.h>

namespace kk::utils {

void TimeProfiler::Point(const std::string &name) {
    tgfx::PrintLog("[TimeProfiler] %s", name.c_str());
}

TimeProfiler::TimeProfiler(const std::string &tag, bool autoStart, bool autoLog, TimeProfilerGroup *group)
    : _tag(tag)
    , _autoLog(autoLog)
    , _started(false)
    , _stopped(false)
    , _group(group) {

    if (autoStart) {
        start();
    }
}

TimeProfiler::~TimeProfiler() {
    if (!_stopped && _started) {
        auto elapsedMs = end();

        if (_autoLog && _group == nullptr) {
            tgfx::PrintLog("[TimeProfiler] %s: %.3f ms", _tag.c_str(), elapsedMs);
        }
    }
}

void TimeProfiler::start() {
    _startTime = std::chrono::high_resolution_clock::now();
    _started = true;
    _stopped = false;
}

double TimeProfiler::end() {
    if (_stopped || !_started) {
        return 0.0;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - _startTime);
    double elapsedMs = duration.count() / 1000.0;
    _stopped = true;

    if (_group != nullptr) {
        recordToGroup(elapsedMs);
    }

    return elapsedMs;
}

void TimeProfiler::recordToGroup(double elapsedMs) {
    if (_group != nullptr) {
        TimeProfilerStage stage;
        stage.name = _tag;
        stage.elapsedMs = elapsedMs;
        _group->_stages.push_back(stage);
    }
}

double TimeProfiler::elapsed() const {
    if (_stopped || !_started) {
        return 0.0;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - _startTime);
    return duration.count() / 1000.0;
}

void TimeProfiler::reset() {
    _started = false;
    _stopped = false;
    start();
}

void ManualTimeProfiler::start() {
    _startTime = std::chrono::high_resolution_clock::now();
    _running = true;
}

double ManualTimeProfiler::stop() {
    if (!_running) {
        return 0.0;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - _startTime);
    _running = false;
    return duration.count() / 1000.0;
}

double ManualTimeProfiler::elapsed() const {
    if (!_running) {
        return 0.0;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - _startTime);
    return duration.count() / 1000.0;
}

void ManualTimeProfiler::reset() {
    _running = false;
}

TimeProfilerGroup::TimeProfilerGroup(const std::string &groupName, bool autoStart, bool autoLog)
    : _groupName(groupName)
    , _autoLog(autoLog)
    , _started(false)
    , _stopped(false) {
    if (autoStart) {
        start();
    }
}

TimeProfilerGroup::~TimeProfilerGroup() {
    if (!_stopped && _started && _autoLog) {
        end();
    }
}

void TimeProfilerGroup::start() {
    _startTime = std::chrono::high_resolution_clock::now();
    _started = true;
    _stopped = false;
}

double TimeProfilerGroup::end() {
    if (_stopped || !_started) {
        return _totalElapsedMs;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - _startTime);
    _totalElapsedMs = duration.count() / 1000.0;
    _stopped = true;

    if (_autoLog) {
        logResults();
    }

    return _totalElapsedMs;
}

TimeProfiler &TimeProfilerGroup::addStage(const std::string &stageName) {
    _activeStages.push_back(std::make_unique<TimeProfiler>(stageName, false, false, this));
    return *_activeStages.back();
}

double TimeProfilerGroup::getTotalElapsed() const {
    if (!_started || _stopped) {
        return 0.0;
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - _startTime);
    return duration.count() / 1000.0;  // 转换为毫秒
}

void TimeProfilerGroup::setDisableAutoLog(bool disable) {
    _autoLog = !disable;
}

void TimeProfilerGroup::setLogThreshold(double thresholdMs) {
    _logThresholdMs = thresholdMs;
}

void TimeProfilerGroup::logResults() const {
    double totalMs = _stopped ? _totalElapsedMs : getTotalElapsed();

    if (_logThresholdMs > 0.0 && totalMs < _logThresholdMs) {
        return;
    }

    tgfx::PrintLog("[TimeProfiler] %s: %.3f ms", _groupName.c_str(), totalMs);

    if (!_stages.empty()) {
        for (size_t i = 0; i < _stages.size(); i++) {
            const auto &stage = _stages[i];
            bool isLast = (i == _stages.size() - 1);
            std::string prefix = isLast ? "  └─ " : "  ├─ ";
            tgfx::PrintLog("[TimeProfiler] %s%s: %.3f ms", prefix.c_str(), stage.name.c_str(), stage.elapsedMs);
        }
    }
}

std::string TimeProfilerGroup::getIndent(int level) const {
    std::string indent;
    for (int i = 0; i < level; i++) {
        indent += "  ";
    }
    return indent;
}

};  // namespace kk::utils

#endif
