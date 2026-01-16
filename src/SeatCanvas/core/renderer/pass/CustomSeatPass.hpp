//
//  CustomSeatPass.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/9.
//

#ifndef CustomSeatPass_hpp
#define CustomSeatPass_hpp

#include "core/renderer/BaseMapRegionVertex.hpp"
#include "core/renderer/pass/CustomRenderPass.hpp"
#include "core/renderer/pass/Uniform.hpp"

#include <tgfx/core/Matrix.h>
namespace kk::renderer {
class UniformData;
struct SeatRegionMesh;
class CustomSeatPass : public CustomRenderPass {
  public:
    CustomSeatPass();
    virtual ~CustomSeatPass();

    void updateRegionMeshes(const std::vector<std::shared_ptr<SeatRegionMesh>> &meshes);
    void updateUVOffset(const std::vector<float> &uvOffset);

    void clearRegionMeshes();
    bool hasData() const;
    void setDefaultColor(const tgfx::Color &color);
    void setAtlasTexture(std::shared_ptr<tgfx::Texture> texture);

    virtual bool onDraw(tgfx::CommandEncoder *encoder, const SeatCanvasCoreRendererState *state) override;
    virtual std::shared_ptr<tgfx::Image> outputImage() override;

  protected:
    virtual void onResetGPUResources() override;
    virtual std::string onBuildVertexShader() const override;

    virtual std::string onBuildFragmentShader() const override;
    virtual std::vector<tgfx::VertexBufferLayout> vertexBufferLayouts() const override;
    virtual std::vector<tgfx::BindingEntry> uniformBlocks() const override;
    std::vector<tgfx::BindingEntry> textureSamplers() const override;

    bool updateMVPMatrix(const SeatCanvasCoreRendererState *state);

    bool prepareRenderTexture(tgfx::GPU *gpu, int width, int height);
    bool preparePipeline(tgfx::GPU *gpu);
    bool prepareSampler(tgfx::GPU *gpu);
    bool prepareBuffer(tgfx::GPU *gpu);
    bool updateUBOBuffer();

  private:
    struct {
        bool dirtyUVTable : 1;
        bool avaiable : 1;
    } bitFields = {};

    std::vector<std::shared_ptr<SeatRegionMesh>> regionMeshes = {};
    std::vector<float> uvOffset = {};
    tgfx::Color defaultColor = tgfx::Color{0.3f, 0.6f, 0.9f, 1.0f};
    tgfx::Matrix mvpMatrix = {tgfx::Matrix::I()};

    std::unique_ptr<UniformData> uniformData = {nullptr};
    std::shared_ptr<tgfx::GPUBuffer> uboBuffer = {nullptr};
    std::shared_ptr<tgfx::RenderPipeline> pipeline = {nullptr};
    std::shared_ptr<tgfx::Image> textureImage = {nullptr};
    std::shared_ptr<tgfx::Texture> renderTexture = {nullptr};
    std::shared_ptr<tgfx::Texture> atlasTexture = {nullptr};
    std::shared_ptr<tgfx::Sampler> sampler = {nullptr};
    tgfx::Attribute position;
    tgfx::Attribute textureCoord;
    tgfx::Attribute styleIndex;

    Uniform mvpUniform;
    Uniform textureCoordRectsUniform;
};
};  // namespace kk::renderer

#endif /* CustomSeatPass_hpp */
