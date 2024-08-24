// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/backends/imgui_impl_glfw.h"
#include "thirdparty/imgui/backends/imgui_impl_opengl3.h"
#include "thirdparty/opengl/glfw/include/GLFW/glfw3.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/EditorUIRenderer.h"
#include "Editor/EditorUI.h"
#include "Core/Render/WindowSystem.h"
#include "glWrapper/RenderEventInfo.h"

namespace Aurora
{

bool EditorUIRenderer::Init()
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

    m_editor_UI = std::make_shared<EditorUI>();
    if (!m_editor_UI->Init()) return false;

    return true;
}

void EditorUIRenderer::Render()
{
    SCOPED_RENDER_EVENT("Editor UI");
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_editor_UI->Layout();
    
    // Rendering
    ImGui::Render();
    auto size = WindowSystem::GetInstance().GetWindowSize();
    glViewport(0, 0, size[0], size[1]);
    glClear(GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorUIRenderer::OnSelectedSceneObjectChange() const
{
    m_editor_UI->OnSelectedSceneObjectChange();
}

bool EditorUIRenderer::OnClose() const
{
    return m_editor_UI->OnClose();
}

EditorUIRenderer::~EditorUIRenderer()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
} // namespace Aurora