#pragma once

#include <gtest/gtest.h>

namespace kk::test {

class TestEnvironment : public ::testing::Environment {
  public:
    void SetUp() override;
    void TearDown() override;
};

};  // namespace kk::test
