//
//  CustomRenderPass.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "CustomRenderPass.hpp"

#include <tgfx/gpu/Context.h>
#include <tgfx/gpu/GPU.h>
#include <tgfx/platform/Print.h>

namespace kk::renderer {

std::shared_ptr<tgfx::RenderPipeline> CustomRenderPass::CreatePipeline(tgfx::GPU *gpu, const std::string &vertexShader, const std::string &fragmentShader, const std::vector<tgfx::VertexBufferLayout> &vertexBufferLayouts, const std::vector<tgfx::BindingEntry> &uniformBlocks, const std::vector<tgfx::PipelineColorAttachment> &colorAttachments, const std::vector<tgfx::BindingEntry> &textureSamplers) {
    auto info = gpu->info();
    auto isDesktop = info->version.find("OpenGL ES") == std::string::npos;
    std::string versionPrefix = isDesktop ? "#version 150\n\n" : "#version 300 es\n\n";
    tgfx::ShaderModuleDescriptor vertexModule = {};
    vertexModule.code = versionPrefix + vertexShader;
    vertexModule.stage = tgfx::ShaderStage::Vertex;
    auto vertexShaderModule = gpu->createShaderModule(vertexModule);
    if (vertexShaderModule == nullptr) {
        tgfx::PrintError("CustomRenderPass: Failed to create vertex shader");
        return nullptr;
    }

    tgfx::ShaderModuleDescriptor fragmentModule = {};
    fragmentModule.code = versionPrefix + fragmentShader;
    fragmentModule.stage = tgfx::ShaderStage::Fragment;
    auto fragmentShaderModule = gpu->createShaderModule(fragmentModule);
    if (fragmentShaderModule == nullptr) {
        tgfx::PrintError("CustomRenderPass: Failed to create fragment shader");
        return nullptr;
    }

    tgfx::RenderPipelineDescriptor descriptor = {};
    descriptor.vertex.bufferLayouts = vertexBufferLayouts;
    descriptor.vertex.module = vertexShaderModule;
    descriptor.fragment.module = fragmentShaderModule;
    descriptor.fragment.colorAttachments = colorAttachments;
    descriptor.layout.textureSamplers = textureSamplers;
    descriptor.layout.uniformBlocks = uniformBlocks;
    return gpu->createRenderPipeline(descriptor);
}

std::shared_ptr<tgfx::RenderPipeline> CustomRenderPass::createPipeline(tgfx::GPU *gpu) const {
    return CreatePipeline(gpu, onBuildVertexShader(), onBuildFragmentShader(), vertexBufferLayouts(), uniformBlocks(), colorAttachments(), textureSamplers());
}

};  // namespace kk::renderer
