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
#include "Core/Render/WindowSystem.h"

namespace Aurora
{

bool EditorUIPass::Init()
{
    GLFWwindow* window = WindowSystem::GetInstance().GetWindow();
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
        if (!window_UI->Init()) return false;
    }

    return true;
}

void EditorUIPass::Render()
{
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
    auto size = WindowSystem::GetInstance().GetWindowSize();
    glViewport(0, 0, size[0], size[1]);
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