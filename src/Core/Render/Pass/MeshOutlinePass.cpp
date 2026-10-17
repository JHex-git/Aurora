// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/MeshOutlinePass.h"
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"
#include "Core/Render/RenderGraph/RenderGraphContext.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/Camera.h"
#include "glWrapper/FrameBufferObject.h"
#include "glWrapper/RenderEventInfo.h"

// Stencil Buffer 1 means selected mesh
namespace Aurora
{

constexpr float outline_thickness = 10.f;

namespace
{
struct OutlineParameter : PassParameterBase
{
    RenderGraphTextureHandle color_in;
    RenderGraphTextureHandle color_out;
    RenderGraphTextureHandle depth_in;
    RenderGraphTextureHandle depth_out;
    int width = 0;
    int height = 0;
};
} // namespace

bool MeshOutlinePass::Init()
{
    {
        std::vector<Shader> shaders;
        shaders.emplace_back(ShaderType::VertexShader);
        auto vert_path = FileSystem::GetFullPath("shaders/mesh.vert");
        shaders[0].SetFlag("ENABLE_NORMALS");
        if (!shaders[0].Load(vert_path))
        {
            spdlog::error("Failed to load vertex shader {}", vert_path);
            return false;
        }
        shaders.emplace_back(ShaderType::FragmentShader);
        auto frag_path = FileSystem::GetFullPath("shaders/pure.frag");
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_mesh_stencil_shader_program = std::make_unique<ShaderProgram>();
        if (!m_mesh_stencil_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    {
        std::vector<Shader> shaders;
        shaders.emplace_back(ShaderType::VertexShader);
        auto vert_path = FileSystem::GetFullPath("shaders/outline.vert");
        if (!shaders[0].Load(vert_path))
        {
            spdlog::error("Failed to load vertex shader {}", vert_path);
            return false;
        }
        shaders.emplace_back(ShaderType::FragmentShader);
        auto frag_path = FileSystem::GetFullPath("shaders/pure.frag");
        if (!shaders[1].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_outline_shader_program = std::make_unique<ShaderProgram>();
        if (!m_outline_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }

    return true;
}

RenderGraphTextureHandle MeshOutlinePass::Render(RenderGraphBuilder& graph_builder,
                                                 RenderGraphTextureHandle color,
                                                 RenderGraphTextureHandle depth,
                                                 RenderGraphTextureHandle& output_depth,
                                                 const std::array<int, 2>& viewport_size)
{
    output_depth = depth;
    if (m_mesh_stencil_shader_program == nullptr || m_outline_shader_program == nullptr) return color;
    auto selected_mesh_render_material = m_selected_mesh_render_material.lock();
    if (selected_mesh_render_material == nullptr) return color;

    OutlineParameter* param = graph_builder.AllocateParameter<OutlineParameter>();
    param->color_in = color;
    param->color_out = graph_builder.CreateAliasedTexture("OutlineColor", color);
    param->depth_in = depth;
    // The pass writes stencil, which lives in the depth-stencil texture.
    param->depth_out = graph_builder.CreateAliasedTexture("OutlineDepth", depth);
    output_depth = param->depth_out;
    param->width = viewport_size[0];
    param->height = viewport_size[1];

    graph_builder.AddPass(
        "MeshOutline",
        [param](RenderGraphPass& pass)
        {
            pass.Consume(param->color_in);
            pass.Produce(param->color_out);
            pass.Consume(param->depth_in);
            pass.Produce(param->depth_out);
        },
        [param, this, selected_mesh_render_material](RenderGraphContext& context)
        {
            FrameBufferObject& fbo = context.GetFrameBuffer({param->color_out}, param->depth_out);
            fbo.Bind();
            const glm::mat4 model = selected_mesh_render_material->GetModelMatrix();
            {
                SCOPED_RENDER_EVENT("Draw mesh stencil");

                RenderState render_state;
                render_state.depth_stencil_state = {
                    .depth_test_enabled = false,
                    .depth_write_enabled = false,
                    .stencil_test_enabled = true,
                    .stencil_write_mask = 0xFF,  // enable writing to the stencil buffer
                    .stencil_func = {GL_ALWAYS, 1, 0xFF},
                    .stencil_op = {GL_KEEP, GL_KEEP, GL_REPLACE} // write 1 to the stencil buffer where the mesh is rendered
                };
                render_state.color_mask = ColorMask::WriteNone();
                context.ApplyRenderState(render_state);
                glClear(GL_STENCIL_BUFFER_BIT);

                // render selected model to write stencil buffer
                m_mesh_stencil_shader_program->Bind();
                m_mesh_stencil_shader_program->SetUniform("uModel", model);
                m_mesh_stencil_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
                m_mesh_stencil_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
                for (size_t i = 0; i < selected_mesh_render_material->m_mesh->m_submeshes.size(); ++i)
                {
                    selected_mesh_render_material->m_vaos[i]->Bind();
                    glDrawElements(GL_TRIANGLES, selected_mesh_render_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                    selected_mesh_render_material->m_vaos[i]->Unbind();
                }
                m_mesh_stencil_shader_program->Unbind();
            }

            {
                SCOPED_RENDER_EVENT("Draw outline");

                RenderState render_state;
                render_state.color_mask = ColorMask::WriteAll();
                render_state.depth_stencil_state = {
                    .depth_test_enabled = false,
                    .depth_write_enabled = true,
                    .stencil_test_enabled = true,
                    .stencil_write_mask = 0x00, // disable writing to the stencil buffer
                    .stencil_func = {GL_NOTEQUAL, 1, 0xFF},
                    .stencil_op = {GL_KEEP, GL_KEEP, GL_KEEP}
        };
        render_state.raster_state.cull_face = GL_FRONT;
        context.ApplyRenderState(render_state);

        // render dilated model and use stencil buffer to render outline
        m_outline_shader_program->Bind();
        m_outline_shader_program->SetUniform("uModel", model);
        m_outline_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
        m_outline_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
        m_outline_shader_program->SetUniform("uOutlineThickness", outline_thickness);
        m_outline_shader_program->SetUniform("uScreenWidth", param->width);
        m_outline_shader_program->SetUniform("uScreenHeight", param->height);
        m_outline_shader_program->SetUniform("uColor", glm::vec3(1.0f, 1.0f, 1.0f));
        for (size_t i = 0; i < selected_mesh_render_material->m_mesh->m_submeshes.size(); ++i)
        {
            selected_mesh_render_material->m_vaos[i]->Bind();
            glDrawElements(GL_TRIANGLES, selected_mesh_render_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
            selected_mesh_render_material->m_vaos[i]->Unbind();
        }
        m_outline_shader_program->Unbind();
            }
        });

    return param->color_out;
}
} // namespace Aurora