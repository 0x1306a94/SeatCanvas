#pragma once

#include <memory>

namespace kk::renderer {
class ApplePlatformView;
};  // namespace kk::renderer

namespace kk::test {

class TestMetalView {
  public:
    explicit TestMetalView(int width, int height, float density);
    ~TestMetalView();

    TestMetalView(const TestMetalView &) = delete;
    TestMetalView &operator=(const TestMetalView &) = delete;

    void *rawView() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> _impl = {nullptr};
};

void waitForGPU(kk::renderer::ApplePlatformView *platformView);

};  // namespace kk::test
