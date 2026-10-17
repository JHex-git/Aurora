// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/Pass/TonemapPass.h"
#include "Utility/FileSystem.h"
#include "glWrapper/Shader.h"
#include "glWrapper/RenderEventInfo.h"
#include "glWrapper/Utils.h"
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"
#include "Core/Render/RenderGraph/RenderGraphContext.h"

namespace Aurora
{
namespace
{
struct TonemapParameter : PassParameterBase
{
    RenderGraphTextureHandle input_color;
    RenderGraphTextureHandle output_color;
};
} // namespace

bool TonemapPass::Init()
{
    std::vector<Shader> shaders;
    shaders.emplace_back(ShaderType::VertexShader);
    auto vert_path = FileSystem::GetFullPath("shaders/tonemap.vert");
    if (!shaders[0].Load(vert_path))
    {
        spdlog::error("Failed to load vertex shader {}", vert_path);
        return false;
    }
    shaders.emplace_back(ShaderType::FragmentShader);
    auto frag_path = FileSystem::GetFullPath("shaders/tonemap.frag");
    if (!shaders[1].Load(frag_path))
    {
        spdlog::error("Failed to load fragment shader {}", frag_path);
        return false;
    }
    m_shader_program = std::make_unique<ShaderProgram>();
    if (!m_shader_program->Load(shaders))
    {
        spdlog::error("Failed to load shader program");
        return false;
    }

    return true;
}

RenderGraphTextureHandle TonemapPass::Render(RenderGraphBuilder& graph_builder,
                                             RenderGraphTextureHandle scene_color,
                                             RenderGraphTextureHandle output,
                                             const std::array<int, 2>& viewport_size)
{
    ShaderProgram* shader = m_shader_program.get();
    if (shader == nullptr) return {};

    TonemapParameter* param = graph_builder.AllocateParameter<TonemapParameter>();
    param->input_color = scene_color;
    RenderGraphTextureHandle tonemap_color = output;
    if (!tonemap_color.IsValid())
    {
        tonemap_color = graph_builder.CreateTexture(
            "TonemapColor",
            {.width = viewport_size[0],
             .height = viewport_size[1],
             .internal_format = GL_RGBA8,
             .format = GL_RGBA,
             .type = GL_UNSIGNED_BYTE});
    }
    param->output_color = tonemap_color;
    graph_builder.AddPass(
        "Tonemap",
        [param](RenderGraphPass& pass)
        {
            pass.Consume(param->input_color);
            pass.Produce(param->output_color);
        },
        [param, shader](RenderGraphContext& context)
        {

            // Bind() already sets the viewport to the framebuffer size.
            FrameBufferObject& fbo = context.GetFrameBuffer({param->output_color});
            fbo.Bind();

            RenderState render_state;
            render_state.depth_stencil_state.depth_test_enabled = false;
            render_state.depth_stencil_state.depth_write_enabled = false;
            render_state.raster_state.cull_enabled = false;
            context.ApplyRenderState(render_state);

            Texture& texture_hdr = context.GetTexture(param->input_color);
            texture_hdr.Bind(0);
            shader->Bind();
            shader->SetUniform("uHDRTexture", 0);
            shader->SetUniform("uExposure", 1.0f);
            DrawQuad();
            shader->Unbind();
        });

    return tonemap_color;
}
} // namespace Aurora
