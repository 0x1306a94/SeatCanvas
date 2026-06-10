#ifndef SeatCanvasTestFixture_hpp
#define SeatCanvasTestFixture_hpp

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "NullRendererDelegate.hpp"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/renderer/SeatCanvasCoreRenderer.hpp"

namespace kk::renderer {
class PlatformView;
};  // namespace kk::renderer

namespace kk::test {

class SeatCanvasTestFixture : public ::testing::Test {
  public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    void SetUp() override;
    void TearDown() override;

    bool loadSVGBaseMap(kk::renderer::SeatCanvasCoreRenderer &renderer, const std::string &relativePath);
    void renderFrame(kk::renderer::SeatCanvasCoreRenderer &renderer);
    bool compareBaseline(const std::string &key);

  protected:
    kk::renderer::PlatformView *platformView = {nullptr};
    std::unique_ptr<kk::renderer::SeatCanvasCoreRenderer> renderer = {nullptr};
    std::shared_ptr<NullRendererDelegate> delegate = {nullptr};

    std::unique_ptr<kk::renderer::SeatCanvasCoreRenderer> makeRenderer(int width = 750, int height = 1334, float density = 2.0f);
};

};  // namespace kk::test

#endif /* SeatCanvasTestFixture_hpp */
