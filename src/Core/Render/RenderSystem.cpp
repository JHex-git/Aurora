// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
// Aurora include
#include "Core/Render/RenderSystem.h"

namespace Aurora
{

bool RenderSystem::Init(GLFWwindow* window)
{
    m_window = window;
    m_editor_ui_pass = std::make_unique<EditorUIPass>();
    m_view_window = std::make_unique<ViewWindow>();
    return m_editor_ui_pass->Init(m_window) && m_view_window->Init(m_window);
}

void RenderSystem::Render()
{
    m_editor_ui_pass->Render();
    m_view_window->Render();
}

void RenderSystem::AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material)
{
    m_view_window->AddMeshRenderMaterial(mesh_render_material);
}
} // namespace Aurora