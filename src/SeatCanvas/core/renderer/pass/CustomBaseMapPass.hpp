//
//  CustomBaseMapPass.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/9.
//

#ifndef CustomBaseMapPass_hpp
#define CustomBaseMapPass_hpp

#include "core/renderer/BaseMapRegionVertex.hpp"
#include "core/renderer/RegionMeshInfo.hpp"
#include "core/renderer/pass/CustomRenderPass.hpp"
#include "core/renderer/pass/Uniform.hpp"

#include <tgfx/core/Matrix.h>
#include <vector>

namespace tgfx {
class GPUBuffer;
};

namespace kk::renderer {
class UniformData;
class BaseMapMeshBuilder;
class CustomBaseMapPass : public CustomRenderPass {
  public:
    // 颜色模式已移除，现在只使用原始颜色
    // 业务代码可以通过 BaseMapMeshBuilder::updateRegionColor 更新区域颜色

    CustomBaseMapPass();
    virtual ~CustomBaseMapPass();

    void updateMeshBuilder(std::shared_ptr<BaseMapMeshBuilder> meshBuilder);

    // 颜色模式相关方法已移除

    virtual bool onDraw(tgfx::CommandEncoder *encoder, const SeatCanvasCoreRendererState *state) override;
    virtual std::shared_ptr<tgfx::Image> outputImage() override;

  protected:
    virtual void onResetGPUResources() override;
    virtual std::string onBuildVertexShader() const override;

    virtual std::string onBuildFragmentShader() const override;
    virtual std::vector<tgfx::VertexBufferLayout> vertexBufferLayouts() const override;
    virtual std::vector<tgfx::BindingEntry> uniformBlocks() const override;
    virtual std::vector<tgfx::BindingEntry> textureSamplers() const override;

    bool updateMVPMatrix(const SeatCanvasCoreRendererState *state);

    bool prepareRenderTexture(tgfx::GPU *gpu, int width, int height);
    bool prepareMSAATexture(tgfx::GPU *gpu, int width, int height);

    bool prepareFillPipeline(tgfx::GPU *gpu);
    bool prepareFillBuffer(tgfx::GPU *gpu);
    bool prepareColorTexture(tgfx::GPU *gpu);
    bool updateFillVBOBuffer();
    bool updateFillUBOBuffer();
    bool updateColorTexture(tgfx::GPU *gpu, const SeatCanvasCoreRendererState *state);
    void renderFill(std::shared_ptr<tgfx::RenderPass> &renderPass, const std::vector<size_t> &visibleIndices);
    float hairlineStrokeAlpha(const SeatCanvasCoreRendererState *state, float strokeWidth) const;

    /// CPU 侧可见性裁剪：获取可见的区域网格信息数组
    /// @param state 渲染状态
    /// @return 可见的区域网格信息数组索引（按原始顺序）
    std::vector<size_t> getVisibleRegionMesheIndices(const SeatCanvasCoreRendererState *state) const;

  private:
    struct {
        bool dirtyFillUBO : 1;
        bool dirtyFillVBO : 1;
        bool dirtyFillColor : 1;
        bool dirtyStrokeUBO : 1;
        bool dirtyStrokeVBO : 1;
        bool avaiable : 1;
    } bitFields = {};

    tgfx::Matrix mvpMatrix = {tgfx::Matrix::I()};

    std::shared_ptr<BaseMapMeshBuilder> meshBuilder = {nullptr};
    std::unique_ptr<UniformData> fillUniformData = {nullptr};
    std::shared_ptr<tgfx::GPUBuffer> fillVBOBuffer = {nullptr};
    std::shared_ptr<tgfx::GPUBuffer> fillUBOBuffer = {nullptr};
    std::shared_ptr<tgfx::RenderPipeline> fillPipeline = {nullptr};
    std::shared_ptr<tgfx::Image> textureImage = {nullptr};
    std::shared_ptr<tgfx::Texture> msaaTexture = {nullptr};
    std::shared_ptr<tgfx::Texture> renderTexture = {nullptr};
    tgfx::Attribute position;
    tgfx::Attribute coverage;
    tgfx::Attribute colorIndex;

    Uniform fillMVPUniform;
    Uniform fillColorTextureSizeUniform;

    std::shared_ptr<tgfx::Texture> colorTexture = {nullptr};
    std::shared_ptr<tgfx::Sampler> colorSampler = {nullptr};
};
};  // namespace kk::renderer

#endif /* CustomBaseMapPass_hpp */
