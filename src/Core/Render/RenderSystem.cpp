// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
// Aurora include
#include "Core/Render/RenderSystem.h"

namespace Aurora
{

bool RenderSystem::Init()
{
    glfwSwapInterval(0); // Enable vsync
    m_editor_ui_renderer = std::make_unique<EditorUIRenderer>();
    m_view_window = std::make_unique<ViewWindow>();
    return m_editor_ui_renderer->Init() && m_view_window->Init();
}

void RenderSystem::Render()
{
    m_editor_ui_renderer->Render();
    m_view_window->Render();
}

MeshRenderMaterialID RenderSystem::RegisterMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material)
{
    return m_view_window->RegisterMeshRenderMaterial(mesh_render_material);
}

void RenderSystem::UnregisterMeshRenderMaterial(MeshRenderMaterialID id)
{
    m_view_window->UnregisterMeshRenderMaterial(id);
}

void RenderSystem::SetSkyboxRenderMaterial(std::shared_ptr<SkyboxRenderMaterial> skybox_render_material)
{
    m_view_window->SetSkyboxRenderMaterial(skybox_render_material);
}
} // namespace Aurora