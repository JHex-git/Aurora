// std include
#include <functional>
// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/imgui/imgui_internal.h"
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
// Aurora include
#include "Core/Render/ViewWindow.h"
#include "Runtime/Scene/Camera.h"
#include "Core/Render/WindowSystem.h"

namespace Aurora
{

bool ViewWindow::Init()
{
    WindowSystem::GetInstance().RegisterMousePosCallback(std::bind(&ViewWindow::OnMousePosCallback, this, std::placeholders::_1, std::placeholders::_2));
    WindowSystem::GetInstance().RegisterMouseButtonCallback(std::bind(&ViewWindow::OnMouseButtonCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    WindowSystem::GetInstance().RegisterScrollCallback(std::bind(&ViewWindow::OnScrollCallback, this, std::placeholders::_1, std::placeholders::_2));
    WindowSystem::GetInstance().RegisterKeyCallback(std::bind(&ViewWindow::OnKeyCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    m_render_pipeline = std::make_unique<RenderPipeline>();
    return m_render_pipeline->Init();
}

void ViewWindow::Render()
{
    glViewport(m_view_info.m_position[0], m_view_info.m_position[1], m_view_info.m_size[0], m_view_info.m_size[1]);
    m_render_pipeline->Render();
}

void ViewWindow::OnMousePosCallback(double xpos, double ypos)
{
    if (!WindowSystem::GetInstance().IsCursorCaptured()) return;
    
    double xoffset = xpos - m_last_x;
    double yoffset = m_last_y - ypos; // reversed since y-coordinates go from bottom to top

    m_last_x = xpos;
    m_last_y = ypos;

    if (WindowSystem::GetInstance().IsMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
        MainCamera::GetInstance().Rotate(xoffset, yoffset);
    else if (WindowSystem::GetInstance().IsMouseButtonDown(GLFW_MOUSE_BUTTON_MIDDLE))
        MainCamera::GetInstance().Tilt(yoffset);
}

void ViewWindow::OnMouseButtonCallback(int button, int action, int mods)
{
    if ((button == GLFW_MOUSE_BUTTON_RIGHT || button == GLFW_MOUSE_BUTTON_MIDDLE) && action == GLFW_RELEASE)
    {
        WindowSystem::GetInstance().SetCursorMode(GLFW_CURSOR_NORMAL);
    }

    auto pos = WindowSystem::GetInstance().GetCursorPos();
    if (pos[0] < m_view_info.m_position[0] || pos[0] > m_view_info.m_position[0] + m_view_info.m_size[0] ||
        pos[1] < m_view_info.m_position[1] || pos[1] > m_view_info.m_position[1] + m_view_info.m_size[1])
        return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT || button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            WindowSystem::GetInstance().SetCursorMode(GLFW_CURSOR_DISABLED);
            m_last_x = pos[0];
            m_last_y = pos[1];
        }
    }
}

void ViewWindow::OnScrollCallback(double xoffset, double yoffset)
{
    if (WindowSystem::GetInstance().IsCursorCaptured())
        MainCamera::GetInstance().AdjustForwardSpeed(yoffset);
}

void ViewWindow::OnKeyCallback(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_A && action == GLFW_REPEAT)
    {
        MainCamera::GetInstance().Pan(false);
    }
    else if (key == GLFW_KEY_D && action == GLFW_REPEAT)
    {
        MainCamera::GetInstance().Pan(true);
    }
    else if (key == GLFW_KEY_W && action == GLFW_REPEAT)
    {
        MainCamera::GetInstance().Dolly(true);
    }
    else if (key == GLFW_KEY_S && action == GLFW_REPEAT)
    {
        MainCamera::GetInstance().Dolly(false);
    }
}
} // namespace Aurora