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
    glfwSwapInterval(1); // Enable vsync
    m_editor_ui_renderer = std::make_unique<EditorUIRenderer>();
    m_view_window = std::make_unique<ViewWindow>();
    return m_editor_ui_renderer->Init() && m_view_window->Init();
}

void RenderSystem::Render()
{
    m_editor_ui_renderer->Render();
    m_view_window->Render();
}

void RenderSystem::AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material)
{
    m_view_window->AddMeshRenderMaterial(mesh_render_material);
}
} // namespace Aurora