// std include

// thirdparty include

// Aurora include
#include "Core/Render/RenderPipeline.h"
#include "Runtime/Scene/SceneManager.h"
#include "Runtime/Scene/Components/MeshRenderer.h"

namespace Aurora
{

bool RenderPipeline::Init()
{
    m_mesh_phong_pass = std::make_unique<MeshPhongPass>();
    m_mesh_outline_pass = std::make_unique<MeshOutlinePass>();
    m_skybox_pass = std::make_unique<SkyboxPass>();
    return m_mesh_phong_pass->Init() && m_mesh_outline_pass->Init() && m_skybox_pass->Init();
}

void RenderPipeline::Render()
{
    m_fbo.Bind();
    glViewport(0, 0, m_render_size[0], m_render_size[1]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_mesh_phong_pass->Render(m_render_size);

    m_skybox_pass->Render(m_render_size);

    // Outline should be rendered after all to ensure not be occluded
    // Use stencil buffer to avoid covering the selected mesh
    auto selected_scene_object = SceneManager::GetInstance().GetScene()->GetSelectedSceneObject();
    if (selected_scene_object != nullptr)
    {
        auto selected_mesh_renderer = selected_scene_object->TryGetComponent<MeshRenderer>();
        if (selected_mesh_renderer != nullptr)
        {
            m_mesh_outline_pass->SetMeshRenderMaterial(selected_mesh_renderer->GetRenderMaterial());
            m_mesh_outline_pass->Render(m_render_size);
        }
    }
    m_fbo.Unbind();
}

void RenderPipeline::AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material)
{
    m_mesh_phong_pass->AddMeshRenderMaterial(mesh_render_material);
}

void RenderPipeline::SetSkyboxRenderMaterial(std::shared_ptr<SkyboxRenderMaterial> skybox_render_material)
{
    m_skybox_pass->SetSkyboxRenderMaterial(skybox_render_material);
}
} // namespace Aurora