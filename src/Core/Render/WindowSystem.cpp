// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "core/render/WindowSystem.h"
#include "Runtime/Scene/Camera.h"

namespace Aurora
{

WindowSystem::~WindowSystem()
{
    Shutdown();
}

bool WindowSystem::Init(unsigned int width, unsigned int height, std::string title)
{
    if (!glfwInit())
    {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (m_window == NULL)
    {
        // std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    m_width = width;
    m_height = height;
    glfwMakeContextCurrent(m_window); // make the window the current context  

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        spdlog::error("Failed to initialize GLAD");
        return false;
    }

    glfwSetWindowUserPointer(m_window, this);

    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset)
    {
        MainCamera::GetInstance().AdjustForwardSpeed(yoffset);
    });

    glfwSetCursorPosCallback(m_window, GLFWCursorPosCallback);
    glfwSetMouseButtonCallback(m_window, GLFWMouseButtonCallback);
    glfwSetKeyCallback(m_window, GLFWKeyCallback);
    
    return true;
}

void WindowSystem::Shutdown() const
{
    glfwTerminate();
}

void WindowSystem::SwapBuffers() const
{
    glfwSwapBuffers(m_window);
}

void WindowSystem::Clear() const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void WindowSystem::PollEvents() const
{
    glfwPollEvents();
}

bool WindowSystem::ShouldClose() const
{
    return glfwWindowShouldClose(m_window);
}
} // namespace Aurora
