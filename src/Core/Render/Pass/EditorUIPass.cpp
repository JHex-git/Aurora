// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/backends/imgui_impl_glfw.h"
#include "thirdparty/imgui/backends/imgui_impl_opengl3.h"
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/Pass/EditorUIPass.h"
#include "Editor/EditorUI.h"

namespace Aurora
{

bool EditorUIPass::Init(GLFWwindow* window)
{
    m_window = window;
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        spdlog::error("Failed to initialize ImGui GLFW binding");
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 130"))
    {
        spdlog::error("Failed to initialize ImGui OpenGL binding");
        return false;
    }

    m_window_UIs.push_back(std::make_shared<EditorUI>());
    for (auto& window_UI : m_window_UIs)
    {
        if (!window_UI->Init(m_window)) return false;
    }

    return true;
}

void EditorUIPass::Render()
{
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto& window_UI : m_window_UIs)
    {
        window_UI->Layout();
    }
    
    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClear(GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

EditorUIPass::~EditorUIPass()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
} // namespace Aurora