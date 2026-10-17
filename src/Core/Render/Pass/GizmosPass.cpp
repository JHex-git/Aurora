// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Pass/GizmosPass.h"
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"
#include "Core/Render/RenderGraph/RenderGraphContext.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/Scene/SceneManager.h"
#include "glWrapper/FrameBufferObject.h"
#include "glWrapper/RenderEventInfo.h"

namespace Aurora
{

namespace
{
struct GizmosParameter : PassParameterBase
{
    RenderGraphTextureHandle color_in;
    RenderGraphTextureHandle color_out;
    RenderGraphTextureHandle depth_in;
    RenderGraphTextureHandle depth_out;
};
} // namespace

bool GizmosPass::Init()
{
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
    translation_handle_mesh->SetRenderProxy(m_translation_handle_material);

    // auto rotation_handle_mesh = std::make_shared<Mesh>();
    // rotation_handle_mesh->Init(SceneManager::GetInstance().GetDummySceneObject());
    // if (!rotation_handle_mesh->Load("assets/models/gizmos/rotation_handle.fbx")) return false;
    // m_rotation_handle_material = std::make_shared<MeshRenderMaterial>(rotation_handle_mesh);
    // m_rotation_handle_material->Init(SceneManager::GetInstance().GetDummySceneObject());
    // rotation_handle_mesh->SetRenderProxy(m_rotation_handle_material);

    // auto scale_handle_mesh = std::make_shared<Mesh>();
    // scale_handle_mesh->Init(SceneManager::GetInstance().GetDummySceneObject());
    // if (!scale_handle_mesh->Load("assets/models/gizmos/scale_handle.fbx")) return false;
    // m_scale_handle_material = std::make_shared<MeshRenderMaterial>(scale_handle_mesh);
    // m_scale_handle_material->Init(SceneManager::GetInstance().GetDummySceneObject());
    // scale_handle_mesh->SetRenderProxy(m_scale_handle_material);

    m_current_handle_material = m_translation_handle_material;

    return true;
}

RenderGraphTextureHandle GizmosPass::Render(RenderGraphBuilder& graph_builder,
                                            RenderGraphTextureHandle color,
                                            RenderGraphTextureHandle depth)
{
    ShaderProgram* shader = m_shader_program.get();
    if (shader == nullptr) return color;

    GizmosParameter* param = graph_builder.AllocateParameter<GizmosParameter>();
    // Draw on top of the existing target: read the previous version and produce the next
    // one, which aliases the same memory. This keeps one producer per resource and lets
    // the graph derive the order from the dependency instead of declaration order.
    param->color_in = color;
    param->color_out = graph_builder.CreateAliasedTexture("SceneColor.Gizmos", color);
    param->depth_in = depth;
    param->depth_out = graph_builder.CreateAliasedTexture("SceneDepth.Gizmos", depth);

    graph_builder.AddPass(
        "Gizmos",
        [param](RenderGraphPass& pass)
        {
            pass.Consume(param->color_in);
            pass.Produce(param->color_out);
            // The pass clears depth so the gizmo stays visible, which is a real write.
            pass.Consume(param->depth_in);
            pass.Produce(param->depth_out);
        },
        [param, this, shader](RenderGraphContext& context)
        {
            auto selected_transform = m_selected_transform.lock();
            if (selected_transform == nullptr) return;

            FrameBufferObject& fbo = context.GetFrameBuffer({param->color_out}, param->depth_out);
            fbo.Bind();
            // Gizmos must stay visible, so the depth of the scene is discarded first.
            glClear(GL_DEPTH_BUFFER_BIT);

            shader->Bind();
            const glm::mat4 model = glm::scale(glm::translate(glm::identity<glm::mat4>(), selected_transform->GetField<glm::vec3>("m_position")), glm::vec3(1e-4f));
            shader->SetUniform("uModel", model);
            shader->SetUniform("uView", MainCamera::GetInstance().GetViewMatrix());
            shader->SetUniform("uProjection", MainCamera::GetInstance().GetProjectionMatrix());
            shader->SetUniform("uViewPos", MainCamera::GetInstance().GetPosition());
            for (size_t i = 0; i < m_current_handle_material->m_mesh->m_submeshes.size(); ++i)
            {
                shader->SetUniform("uColor", m_current_handle_material->m_mesh->m_submeshes[i].m_color);
                m_current_handle_material->m_vaos[i]->Bind();
                glDrawElements(GL_TRIANGLES, m_current_handle_material->m_mesh->m_submeshes[i].m_indices.size(), GL_UNSIGNED_INT, nullptr);
                m_current_handle_material->m_vaos[i]->Unbind();
            }
            shader->Unbind();
        });

    return param->color_out;
}
} // namespace Aurora