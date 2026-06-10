#include "Log.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <tgfx/platform/Print.h>

namespace kk {

static LogLevel gLogLevel = {};
static bool gLogLevelInitialized = false;

static LogLevel DefaultLogLevel() {
#if defined(NDEBUG)
    return LogLevel::Error;
#else
    return LogLevel::Trace;
#endif
}

static LogLevel ParseLogLevel(const char *value) {
    if (value == nullptr || value[0] == '\0') {
        return DefaultLogLevel();
    }
    if (std::strcmp(value, "error") == 0) {
        return LogLevel::Error;
    }
    if (std::strcmp(value, "warning") == 0 || std::strcmp(value, "warn") == 0) {
        return LogLevel::Warning;
    }
    if (std::strcmp(value, "info") == 0) {
        return LogLevel::Info;
    }
    if (std::strcmp(value, "debug") == 0) {
        return LogLevel::Debug;
    }
    if (std::strcmp(value, "trace") == 0) {
        return LogLevel::Trace;
    }
    return DefaultLogLevel();
}

LogLevel Log::level() {
    if (!gLogLevelInitialized) {
        gLogLevel = ParseLogLevel(std::getenv("SEATCANVAS_LOG_LEVEL"));
        gLogLevelInitialized = true;
    }
    return gLogLevel;
}

void Log::setLevel(LogLevel level) {
    gLogLevel = level;
    gLogLevelInitialized = true;
}

bool Log::isEnabled(LogLevel level) {
    return static_cast<int>(level) <= static_cast<int>(Log::level());
}

void Log::log(LogLevel level, const char *format, ...) {
    if (!isEnabled(level)) {
        return;
    }
    va_list args = {};
    va_start(args, format);
    char buffer[2048] = {};
    std::vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (level == LogLevel::Error) {
        tgfx::PrintError("%s", buffer);
    } else {
        tgfx::PrintLog("%s", buffer);
    }
}

}  // namespace kk
