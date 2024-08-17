// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/GizmosPass.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/Scene/SceneManager.h"

namespace Aurora
{

bool GizmosPass::Init(const std::array<int, 2>& viewport_size)
{
    if (!RenderPass::Init(viewport_size)) return false;
    
    auto fbo = FrameBufferObjectBuilder(viewport_size[0], viewport_size[1])
                                        .AddColorAttachment({})
                                        .EnableDepthAttachment({}).Create();
    if (!fbo.has_value()) return false;
    m_fbo = std::make_shared<FrameBufferObject>(std::move(fbo.value()));

    std::vector<Shader> shaders;
    shaders.emplace_back(ShaderType::VertexShader);
    auto vert_shader_path = FileSystem::GetFullPath("shaders/mesh.vert");
    shaders[0].SetFlag("ENABLE_NORMALS");
    if (!shaders[0].Load(vert_shader_path))
    {
        spdlog::error("Failed to load vertex shader {}", vert_shader_path);
        return false;
    }
    shaders.emplace_back(ShaderType::FragmentShader);
    auto frag_shader_path = FileSystem::GetFullPath("shaders/mesh_nov.frag");
    if (!shaders[1].Load(frag_shader_path))
    {
        spdlog::error("Failed to load fragment shader {}", frag_shader_path);
        return false;
    }
    m_shader_program = std::make_unique<ShaderProgram>();
    if (!m_shader_program->Load(shaders))
    {
        spdlog::error("Failed to load shader program");
        return false;
    }

    auto translation_handle_mesh = std::make_shared<Mesh>();
    translation_handle_mesh->Init(SceneManager::GetInstance().GetDummySceneObject());
    if (!translation_handle_mesh->Load("assets/models/gizmos/translation_handle.fbx")) return false;
    m_translation_handle_material = std::make_shared<MeshRenderMaterial>(translation_handle_mesh);
    m_translation_handle_material->Init(SceneManager::GetInstance().GetDummySceneObject());

    // auto rotation_handle_mesh = std::make_shared<Mesh>();
    // rotation_handle_mesh->Init(SceneManager::GetInstance().GetDummySceneObject());
    // if (!rotation_handle_mesh->Load("assets/models/gizmos/rotation_handle.fbx")) return false;
    // m_rotation_handle_material = std::make_shared<MeshRenderMaterial>(rotation_handle_mesh);
    // m_rotation_handle_material->Init(SceneManager::GetInstance().GetDummySceneObject());

    // auto scale_handle_mesh = std::make_shared<Mesh>();
    // scale_handle_mesh->Init(SceneManager::GetInstance().GetDummySceneObject());
    // if (!scale_handle_mesh->Load("assets/models/gizmos/scale_handle.fbx")) return false;
    // m_scale_handle_material = std::make_shared<MeshRenderMaterial>(scale_handle_mesh);
    // m_scale_handle_material->Init(SceneManager::GetInstance().GetDummySceneObject());

    m_current_handle_material = m_translation_handle_material;

    return true;
}

void GizmosPass::Render()
{
    m_fbo->Bind();
    glViewport(0, 0, m_viewport_size[0], m_viewport_size[1]);
    glClear(GL_DEPTH_BUFFER_BIT);
    if (m_shader_program == nullptr) return;


    // render handle
    if (auto selected_transform = m_selected_transform.lock())
    {
        m_shader_program->Bind();
        const glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), selected_transform->GetField<glm::vec3>("m_position")), glm::vec3(1e-4));
        m_shader_program->SetUniform("uModel", model);
        m_shader_program->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
        m_shader_program->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
        m_shader_program->SetUniform("uViewPos", MainCamera::GetInstance().GetPosition());
        for (size_t i = 0; i < m_current_handle_material->m_mesh->m_submeshes.size(); ++i)
        {
            m_shader_program->SetUniform("uColor", m_current_handle_material->m_mesh->m_submeshes[i].m_color);
            m_current_handle_material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)});
            m_current_handle_material->m_vbos[i]->SetAttribPointer(VertexAttribPointer{1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)});

            m_current_handle_material->m_vbos[i]->Bind();
            m_current_handle_material->m_ebos[i]->Bind();
            glDrawElements(GL_TRIANGLES, m_current_handle_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
            m_current_handle_material->m_ebos[i]->Unbind();
            m_current_handle_material->m_vbos[i]->Unbind();
        }
        m_shader_program->Unbind();
    }
    m_fbo->Unbind();
}
} // namespace Aurora