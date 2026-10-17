// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/SkyboxPass.h"
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"
#include "Core/Render/RenderGraph/RenderGraphContext.h"
#include "Utility/FileSystem.h"
#include "glWrapper/Shader.h"
#include "glWrapper/FrameBufferObject.h"
#include "glWrapper/RenderEventInfo.h"
#include "Runtime/Scene/Camera.h"

namespace Aurora
{

namespace
{
struct SkyboxParameter : PassParameterBase
{
    RenderGraphTextureHandle color_in;
    RenderGraphTextureHandle color_out;
    RenderGraphTextureHandle depth;
};
} // namespace

bool SkyboxPass::Init()
{
    std::vector<Shader> shaders;
    shaders.emplace_back(ShaderType::VertexShader);
    if (!shaders[0].Load(FileSystem::GetFullPath("shaders/skybox.vert")))
    {
        spdlog::error("Failed to load vertex shader {}", FileSystem::GetFullPath("shaders/skybox.vert"));
        return false;
    }
    shaders.emplace_back(ShaderType::FragmentShader);
    if (!shaders[1].Load(FileSystem::GetFullPath("shaders/skybox.frag")))
    {
        spdlog::error("Failed to load fragment shader {}", FileSystem::GetFullPath("shaders/skybox.frag"));
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

RenderGraphTextureHandle SkyboxPass::Render(RenderGraphBuilder& graph_builder,
                                            RenderGraphTextureHandle color,
                                            RenderGraphTextureHandle depth)
{
    auto skybox_render_material = m_skybox_render_material.lock();
    ShaderProgram* shader = m_shader_program.get();
    if (skybox_render_material == nullptr || shader == nullptr) return color;

    SkyboxParameter* param = graph_builder.AllocateParameter<SkyboxParameter>();
    param->color_in = color;
    param->color_out = graph_builder.CreateAliasedTexture("SceneColor.Skybox", color);
    param->depth = depth;

    graph_builder.AddPass(
        "Skybox",
        [param](RenderGraphPass& pass)
        {
            pass.Consume(param->color_in);
            pass.Produce(param->color_out);
            pass.Consume(param->depth);
        },
        [param, shader, skybox_render_material](RenderGraphContext& context)
        {
            FrameBufferObject& fbo = context.GetFrameBuffer({param->color_out}, param->depth);
            fbo.Bind();

            RenderState render_state;
            render_state.depth_stencil_state.depth_func = GL_LEQUAL;
            render_state.raster_state.cull_face = GL_FRONT;
            context.ApplyRenderState(render_state);

            shader->Bind();
            // render the loaded model
            const glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), MainCamera::GetInstance().GetPosition());
            shader->SetUniform("uMVP", MainCamera::GetInstance().GetProjectionMatrix() * MainCamera::GetInstance().GetViewMatrix() * model);
            for (size_t i = 0; i < skybox_render_material->m_mesh->m_submeshes.size(); ++i)
            {
                skybox_render_material->m_vaos[i]->Bind();
                skybox_render_material->GetCubemapTexture()->Bind();
                glDrawElements(GL_TRIANGLES, skybox_render_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                skybox_render_material->m_vaos[i]->Unbind();
            }
            shader->Unbind();
        });

    return param->color_out;
}
} // namespace Aurora
