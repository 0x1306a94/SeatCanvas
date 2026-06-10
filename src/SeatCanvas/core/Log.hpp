#pragma once

namespace kk {

enum class LogLevel : int {
    Error = 0,
    Warning = 1,
    Info = 2,
    Debug = 3,
    Trace = 4,
};

class Log {
  public:
    static LogLevel level();
    static void setLevel(LogLevel level);
    static bool isEnabled(LogLevel level);
    static void log(LogLevel level, const char *format, ...);
};

}  // namespace kk

#define SC_LOG_ERROR(...) ::kk::Log::log(::kk::LogLevel::Error, __VA_ARGS__)
#define SC_LOG_WARN(...) ::kk::Log::log(::kk::LogLevel::Warning, __VA_ARGS__)
#define SC_LOG_INFO(...) ::kk::Log::log(::kk::LogLevel::Info, __VA_ARGS__)
#define SC_LOG_DEBUG(...) ::kk::Log::log(::kk::LogLevel::Debug, __VA_ARGS__)
#define SC_LOG_TRACE(function) ::kk::Log::log(::kk::LogLevel::Trace, "%s", function)
