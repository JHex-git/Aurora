// std include

// thirdparty include

// Aurora include
#include "Core/Render/RenderPipeline.h"
#include "Runtime/Scene/SceneManager.h"
#include "Runtime/Scene/MeshRenderer.h"

namespace Aurora
{

bool RenderPipeline::Init()
{
    m_mesh_phong_pass = std::make_unique<MeshPhongPass>();
    m_mesh_outline_pass = std::make_unique<MeshOutlinePass>();
    return m_mesh_phong_pass->Init() && m_mesh_outline_pass->Init();
}

void RenderPipeline::Render(const std::array<int, 2>& viewport_size)
{
    auto selected_scene_object = SceneManager::GetInstance().GetScene()->GetSelectedSceneObject();
    if (selected_scene_object != nullptr)
    {
        auto selected_mesh_renderer = selected_scene_object->TryGetComponent<MeshRenderer>();
        if (selected_mesh_renderer != nullptr)
        {
            m_mesh_outline_pass->SetMeshRenderMaterial(selected_mesh_renderer->GetRenderMaterial());
            m_mesh_outline_pass->Render(viewport_size);
        }
    }
    m_mesh_phong_pass->Render(viewport_size);
}

void RenderPipeline::AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material)
{
    m_mesh_phong_pass->AddMeshRenderMaterial(mesh_render_material);
}
} // namespace Aurora