#ifndef HeadlessTestPlatformView_hpp
#define HeadlessTestPlatformView_hpp

#include "core/renderer/PlatformView.hpp"

namespace tgfx {
class MetalDevice;
};  // namespace tgfx

namespace kk::test {

class HeadlessTestPlatformView : public kk::renderer::PlatformView {
  public:
    HeadlessTestPlatformView(int width, int height, float density);
    ~HeadlessTestPlatformView() override;

    std::shared_ptr<tgfx::Device> getDevice() override;
    std::shared_ptr<tgfx::Window> getWindow() override;
    std::shared_ptr<tgfx::Surface> getSurface(tgfx::Context *context) override;
    void invalidSize() override;
    tgfx::ISize getSize() override;
    float getDensity() override;

  private:
    std::shared_ptr<tgfx::MetalDevice> _device{nullptr};
    std::shared_ptr<tgfx::Surface> _surface{nullptr};
    tgfx::ISize _size{0, 0};
    float _density{1.0f};
};

};  // namespace kk::test

#endif /* HeadlessTestPlatformView_hpp */
