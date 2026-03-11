// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/Pass/TonemapPass.h"
#include "Utility/FileSystem.h"
#include "glWrapper/Shader.h"
#include "glWrapper/RenderEventInfo.h"
#include "glWrapper/Utils.h"

namespace Aurora
{

bool TonemapPass::Init(const std::array<int, 2>& viewport_size)
{
    if (!RenderPass::Init(viewport_size)) return false;

    auto fbo = FrameBufferObjectBuilder(viewport_size[0], viewport_size[1])
                                        .AddColorAttachment({.internal_format = GL_RGBA, .format = GL_RGBA, .type = GL_UNSIGNED_BYTE})
                                        .EnableDepthAttachment({}).Create();
    if (!fbo.has_value()) return false;
    m_fbo = std::make_shared<FrameBufferObject>(std::move(fbo.value()));

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

void TonemapPass::Render(ContextState& context_state)
{
    if (m_shader_program == nullptr || m_source_fbo == nullptr) return;

    SCOPED_RENDER_EVENT("Tonemap Pass");
    m_fbo->Bind();
    glViewport(0, 0, m_viewport_size[0], m_viewport_size[1]);

    RenderState render_state;
    render_state.depth_stencil_state.depth_test_enabled = false;
    render_state.depth_stencil_state.depth_write_enabled = false;
    render_state.raster_state.cull_enabled = false;
    context_state.ApplyRenderState(render_state);

    m_shader_program->Bind();
    m_source_fbo->BindColorTexture(0, 0);
    m_shader_program->SetUniform("uHDRTexture", 0);
    m_shader_program->SetUniform("uExposure", 1.0f);
    DrawQuad(m_source_fbo->GetColorAttachmentID(0));
    m_shader_program->Unbind();

    m_fbo->Unbind();
}
} // namespace Aurora
