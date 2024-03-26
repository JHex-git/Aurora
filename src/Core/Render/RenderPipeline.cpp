// std include

// thirdparty include

// Aurora include
#include "Core/Render/RenderPipeline.h"

namespace Aurora
{

bool RenderPipeline::Init(GLFWwindow* window)
{
    m_window = window;
    m_mesh_phong_pass = std::make_unique<MeshPhongPass>();
    return m_mesh_phong_pass->Init(m_window);
}

void RenderPipeline::Render()
{
    m_mesh_phong_pass->Render();
}

void RenderPipeline::AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material)
{
    m_mesh_phong_pass->AddMeshRenderMaterial(mesh_render_material);
}
} // namespace Aurora