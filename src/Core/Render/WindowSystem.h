#pragma once
// std include
#include <string>
#include <array>
// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
// Aurora include

struct GLFWwindow;
namespace Aurora
{

class WindowSystem
{
public:
    static WindowSystem& GetInstance()
    {
        static WindowSystem instance;
        return instance;
    }

    WindowSystem(const WindowSystem&) = delete;
    WindowSystem& operator=(const WindowSystem&) = delete;

    WindowSystem(WindowSystem&&) = delete;
    WindowSystem& operator=(WindowSystem&&) = delete;

    ~WindowSystem();

    bool Init(unsigned int width, unsigned int height, std::string title);
    void Shutdown() const;

    void SwapBuffers() const;
    void Clear() const;

    void PollEvents() const;

    bool ShouldClose() const;

    void SetCursorMode(int mode) const { glfwSetInputMode(m_window, GLFW_CURSOR, mode); }

    const std::array<unsigned int, 2> GetWindowSize() const { return { m_width, m_height }; }
    const std::array<double, 2> GetCursorPos() const
    {
        double xpos, ypos;
        glfwGetCursorPos(m_window, &xpos, &ypos);
        return { xpos, ypos };
    }
    bool IsCursorCaptured() const { return glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED; }
    bool IsMouseButtonDown(int button) const { return glfwGetMouseButton(m_window, button) == GLFW_PRESS; }

    GLFWwindow* GetWindow() const { return m_window; }

    using CursorPosCallback = std::function<void(double, double)>;
    using MouseButtonCallback = std::function<void(int, int, int)>;
    using ScrollCallback = std::function<void(double, double)>;
    using KeyCallback = std::function<void(int, int, int, int)>;

    inline void RegisterMousePosCallback(CursorPosCallback callback) { m_mouse_pos_callbacks.push_back(callback); }
    inline void RegisterMouseButtonCallback(MouseButtonCallback callback) { m_mouse_button_callbacks.push_back(callback); }
    inline void RegisterScrollCallback(ScrollCallback callback) { m_scroll_callbacks.push_back(callback); }
    inline void RegisterKeyCallback(KeyCallback callback) { m_key_callbacks.push_back(callback); }

private:
    WindowSystem() = default;

    void OnCursorPosEvent(double xpos, double ypos)
    {
        for (const auto& callback : m_mouse_pos_callbacks)
        {
            callback(xpos, ypos);
        }
    }

    void OnMousButtonEvent(int button, int action, int mods)
    {
        for (const auto& callback : m_mouse_button_callbacks)
        {
            callback(button, action, mods);
        }
    }

    void OnScrollEvent(double xoffset, double yoffset)
    {
        for (const auto& callback : m_scroll_callbacks)
        {
            callback(xoffset, yoffset);
        }
    }

    void OnKeyEvent(int key, int scancode, int action, int mods)
    {
        for (const auto& callback : m_key_callbacks)
        {
            callback(key, scancode, action, mods);
        }
    }

    static void GLFWCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        WindowSystem* window_system= static_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
        window_system->OnCursorPosEvent(xpos, ypos);
    }

    static void GLFWMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        WindowSystem* window_system = static_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
        window_system->OnMousButtonEvent(button, action, mods);
    }

    static void GLFWScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        WindowSystem* window_system = static_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
        window_system->OnScrollEvent(xoffset, yoffset);
    }

    static void GLFWKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        WindowSystem* window_system = static_cast<WindowSystem*>(glfwGetWindowUserPointer(window));
        window_system->OnKeyEvent(key, scancode, action, mods);
    }

private:
    std::vector<CursorPosCallback> m_mouse_pos_callbacks;
    std::vector<MouseButtonCallback> m_mouse_button_callbacks;
    std::vector<ScrollCallback> m_scroll_callbacks;
    std::vector<KeyCallback> m_key_callbacks;

    unsigned int m_width;
    unsigned int m_height;
    std::string m_title;
    GLFWwindow* m_window;
};
} // namespace Aurora
