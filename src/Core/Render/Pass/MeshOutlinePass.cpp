// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/MeshOutlinePass.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/Camera.h"
#include "glWrapper/RenderEventInfo.h"

// Stencil Buffer 1 means selected mesh
namespace Aurora
{

constexpr float outline_thickness = 10.f;
bool MeshOutlinePass::Init(const std::array<int, 2>& viewport_size)
{
    if (!RenderPass::Init(viewport_size)) return false;
    
    auto fbo = FrameBufferObjectBuilder(viewport_size[0], viewport_size[1])
                                        .AddColorAttachment({})
                                        .EnableDepthAttachment({.has_stencil = true}).Create();
    if (!fbo.has_value()) return false;
    m_fbo = std::make_shared<FrameBufferObject>(std::move(fbo.value()));

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

void MeshOutlinePass::Render(ContextState& context_state)
{
    if (m_mesh_stencil_shader_program == nullptr || m_outline_shader_program == nullptr) return;
    auto selected_mesh_render_material = m_selected_mesh_render_material.lock();
    if (selected_mesh_render_material == nullptr) return;
    
    SCOPED_RENDER_EVENT("Mesh Outline Pass");
    m_fbo->Bind();
    glViewport(0, 0, m_viewport_size[0], m_viewport_size[1]);
    glClear(GL_STENCIL_BUFFER_BIT);
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
        context_state.ApplyRenderState(render_state);

        // render selected model to write stencil buffer
        m_mesh_stencil_shader_program->Bind();
        m_mesh_stencil_shader_program->SetUniform("uModel", model);
        m_mesh_stencil_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
        m_mesh_stencil_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
        for (size_t i = 0; i < selected_mesh_render_material->m_mesh->m_submeshes.size(); ++i)
        {
            selected_mesh_render_material->m_vbos[i]->Bind();
            selected_mesh_render_material->m_ebos[i]->Bind();
            glDrawElements(GL_TRIANGLES, selected_mesh_render_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
            selected_mesh_render_material->m_ebos[i]->Unbind();
            selected_mesh_render_material->m_vbos[i]->Unbind();
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
        context_state.ApplyRenderState(render_state);

        // render dilated model and use stencil buffer to render outline
        m_outline_shader_program->Bind();
        m_outline_shader_program->SetUniform("uModel", model);
        m_outline_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
        m_outline_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
        m_outline_shader_program->SetUniform("uOutlineThickness", outline_thickness);
        m_outline_shader_program->SetUniform("uScreenWidth", m_viewport_size[0]);
        m_outline_shader_program->SetUniform("uScreenHeight", m_viewport_size[1]);
        m_outline_shader_program->SetUniform("uColor", glm::vec3(1.0f, 1.0f, 1.0f));
        for (size_t i = 0; i < selected_mesh_render_material->m_mesh->m_submeshes.size(); ++i)
        {
            selected_mesh_render_material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)});
            selected_mesh_render_material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)});

            selected_mesh_render_material->m_vbos[i]->Bind();
            selected_mesh_render_material->m_ebos[i]->Bind();
            glDrawElements(GL_TRIANGLES, selected_mesh_render_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
            selected_mesh_render_material->m_ebos[i]->Unbind();
            selected_mesh_render_material->m_vbos[i]->Unbind();
        }
        m_outline_shader_program->Unbind();
    }

    m_fbo->Unbind();
}
} // namespace Aurora