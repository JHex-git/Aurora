#pragma once
// std include
#include <memory>
#include <array>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/EditorUIPass.h"
#include "Core/Render/ViewWindow.h"

class GLFWWindow;
namespace Aurora
{

class RenderSystem
{
public:
    static RenderSystem& GetInstance()
    {
        static RenderSystem instance;
        return instance;
    }

    ~RenderSystem() = default;

    bool Init(GLFWwindow* window);
    void Render();

    void UpdateViewInfo(ViewInfo&& view_info) { m_view_window->UpdateViewInfo(std::move(view_info)); }
    void AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material);

private:
    RenderSystem() = default;

private:
    std::unique_ptr<EditorUIPass> m_editor_ui_pass;
    std::unique_ptr<ViewWindow> m_view_window;
    GLFWwindow* m_window = nullptr;
};
} // namespace Aurora