#include "core/Log.hpp"

#include <gtest/gtest.h>

using namespace kk;

class LogTest : public ::testing::Test {
  public:
    void SetUp() override {
        // Reset to a known state before each test
        Log::setLevel(LogLevel::Trace);
    }
};

TEST_F(LogTest, SetAndGetLevel) {
    Log::setLevel(LogLevel::Error);
    EXPECT_EQ(Log::level(), LogLevel::Error);

    Log::setLevel(LogLevel::Warning);
    EXPECT_EQ(Log::level(), LogLevel::Warning);

    Log::setLevel(LogLevel::Info);
    EXPECT_EQ(Log::level(), LogLevel::Info);

    Log::setLevel(LogLevel::Debug);
    EXPECT_EQ(Log::level(), LogLevel::Debug);

    Log::setLevel(LogLevel::Trace);
    EXPECT_EQ(Log::level(), LogLevel::Trace);
}

TEST_F(LogTest, IsEnabledWhenLevelIsHigherOrEqual) {
    Log::setLevel(LogLevel::Warning);

    EXPECT_TRUE(Log::isEnabled(LogLevel::Error));    // Error < Warning
    EXPECT_TRUE(Log::isEnabled(LogLevel::Warning));  // equal
    EXPECT_FALSE(Log::isEnabled(LogLevel::Info));    // Info > Warning
    EXPECT_FALSE(Log::isEnabled(LogLevel::Debug));   // Debug > Warning
    EXPECT_FALSE(Log::isEnabled(LogLevel::Trace));   // Trace > Warning
}

TEST_F(LogTest, IsEnabledTraceEnablesAll) {
    Log::setLevel(LogLevel::Trace);

    EXPECT_TRUE(Log::isEnabled(LogLevel::Error));
    EXPECT_TRUE(Log::isEnabled(LogLevel::Warning));
    EXPECT_TRUE(Log::isEnabled(LogLevel::Info));
    EXPECT_TRUE(Log::isEnabled(LogLevel::Debug));
    EXPECT_TRUE(Log::isEnabled(LogLevel::Trace));
}

TEST_F(LogTest, IsEnabledErrorEnablesOnlyError) {
    Log::setLevel(LogLevel::Error);

    EXPECT_TRUE(Log::isEnabled(LogLevel::Error));
    EXPECT_FALSE(Log::isEnabled(LogLevel::Warning));
    EXPECT_FALSE(Log::isEnabled(LogLevel::Info));
    EXPECT_FALSE(Log::isEnabled(LogLevel::Debug));
    EXPECT_FALSE(Log::isEnabled(LogLevel::Trace));
}

TEST_F(LogTest, LogDoesNotCrash) {
    Log::log(LogLevel::Error, "test error %d", 42);
    Log::log(LogLevel::Warning, "test warning %s", "hello");
    Log::log(LogLevel::Info, "test info");
    Log::log(LogLevel::Debug, "test debug");
    Log::log(LogLevel::Trace, "test trace");
    SUCCEED();
}

TEST_F(LogTest, LogSuppressedWhenAboveLevel) {
    Log::setLevel(LogLevel::Error);
    // These should be suppressed (no crash is the test)
    Log::log(LogLevel::Warning, "should be suppressed");
    Log::log(LogLevel::Info, "should be suppressed");
    Log::log(LogLevel::Debug, "should be suppressed");
    Log::log(LogLevel::Trace, "should be suppressed");
    SUCCEED();
}

TEST_F(LogTest, MacroCompiles) {
    SC_LOG_ERROR("error macro %d", 1);
    SC_LOG_WARN("warn macro %d", 2);
    SC_LOG_INFO("info macro %d", 3);
    SC_LOG_DEBUG("debug macro %d", 4);
    SC_LOG_TRACE("trace macro");
    SUCCEED();
}
