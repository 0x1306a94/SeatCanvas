//
//  CustomRenderPass.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef CustomRenderPass_hpp
#define CustomRenderPass_hpp

#include <memory>

#include <tgfx/gpu/CommandEncoder.h>

#include "core/renderer/IContextAware.hpp"

namespace tgfx {
class Image;
class Context;
};  // namespace tgfx

namespace kk::renderer {
class SeatCanvasCoreRendererState;

class CustomRenderPass : public IContextAware {
  public:
    virtual ~CustomRenderPass() = default;

    /// 执行渲染
    /// @param encoder CommandEncoder，用于创建和管理 RenderPass
    /// @param state 渲染状态
    /// @return 是否成功渲染
    /// @note 子类应该在此方法中：
    ///       1. 通过 encoder->beginRenderPass() 创建 RenderPass
    ///       2. 设置管线、uniform、纹理、顶点缓冲区等
    ///       3. 执行绘制调用（draw/drawIndexed）
    ///       4. 调用 renderPass->end() 结束 RenderPass
    virtual bool onDraw(tgfx::CommandEncoder *encoder, const SeatCanvasCoreRendererState *state) = 0;

    virtual std::shared_ptr<tgfx::Image> outputImage() = 0;

  protected:
    static std::shared_ptr<tgfx::RenderPipeline> CreatePipeline(tgfx::GPU *gpu, const std::string &vertexShader, const std::string &fragmentShader, const std::vector<tgfx::VertexBufferLayout> &vertexBufferLayouts, const std::vector<tgfx::BindingEntry> &uniformBlocks, const std::vector<tgfx::PipelineColorAttachment> &colorAttachments, const std::vector<tgfx::BindingEntry> &textureSamplers, const tgfx::MultisampleDescriptor &multisample);

    std::shared_ptr<tgfx::RenderPipeline> createPipeline(tgfx::GPU *gpu) const;

    virtual std::string onBuildVertexShader() const = 0;

    virtual std::string onBuildFragmentShader() const = 0;

    virtual std::vector<tgfx::VertexBufferLayout> vertexBufferLayouts() const {
        return {};
    }

    virtual std::vector<tgfx::BindingEntry> uniformBlocks() const {
        return {};
    }

    virtual std::vector<tgfx::BindingEntry> textureSamplers() const {
        return {};
    }

    virtual tgfx::MultisampleDescriptor multisample() const {
        return {};
    }

    virtual std::vector<tgfx::PipelineColorAttachment> colorAttachments() const {
        tgfx::PipelineColorAttachment colorAttachment = {};
        colorAttachment.blendEnable = true;
        colorAttachment.srcColorBlendFactor = tgfx::BlendFactor::One;
        colorAttachment.dstColorBlendFactor = tgfx::BlendFactor::OneMinusSrcAlpha;
        colorAttachment.srcAlphaBlendFactor = tgfx::BlendFactor::One;
        colorAttachment.dstAlphaBlendFactor = tgfx::BlendFactor::OneMinusSrcAlpha;
        return {colorAttachment};
    }
};

};  // namespace kk::renderer

#endif /* CustomRenderPass_hpp */
