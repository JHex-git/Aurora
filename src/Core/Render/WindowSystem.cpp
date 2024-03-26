// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
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

    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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

    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset)
    {
        MainCamera::GetInstance().AdjustForwardSpeed(yoffset);
    });

    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
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
        else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) // TODO: temp
        {
            glfwSetWindowShouldClose(window, true);
        }
    });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos)
    {
        static double lastX = xpos;
        static double lastY = ypos;
        static bool first_mouse;

        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
        {
            lastX = xpos;
            lastY = ypos;
            first_mouse = true;
            return;
        }
        if (first_mouse)
        {
            lastX = xpos;
            lastY = ypos;
            first_mouse = false;
        }

        double xoffset = xpos - lastX;
        double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            MainCamera::GetInstance().Rotate(xoffset, yoffset);
        else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
            MainCamera::GetInstance().Tilt(yoffset);
    });

    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
    {
        if ((button == GLFW_MOUSE_BUTTON_RIGHT || button == GLFW_MOUSE_BUTTON_MIDDLE) && action == GLFW_PRESS)
        {
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if ((button == GLFW_MOUSE_BUTTON_RIGHT || button == GLFW_MOUSE_BUTTON_MIDDLE) && action == GLFW_RELEASE)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        }
    });
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
