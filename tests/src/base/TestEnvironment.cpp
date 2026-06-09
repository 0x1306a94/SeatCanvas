#include "TestEnvironment.hpp"

#include "Baseline.hpp"

namespace kk::test {

void TestEnvironment::SetUp() {
    Baseline::SetUp();
}

void TestEnvironment::TearDown() {
    Baseline::TearDown();
}

};  // namespace kk::test
