// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/imgui/imgui_internal.h"
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
// Aurora include
#include "Core/Render/ViewWindow.h"
#include "Runtime/Scene/Camera.h"

namespace Aurora
{

bool ViewWindow::Init(GLFWwindow* window)
{
    m_window = window;

    m_render_pipeline = std::make_unique<RenderPipeline>();
    return m_render_pipeline->Init(m_window);
}

void ViewWindow::Render()
{
    glViewport(m_view_info.m_position[0], m_view_info.m_position[1], m_view_info.m_size[0], m_view_info.m_size[1]);
    m_render_pipeline->Render();
}
} // namespace Aurora