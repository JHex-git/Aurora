#pragma once
// std include
#include <memory>
#include <array>
// thirdparty include

// Aurora include
#include "Core/Render/EditorUIRenderer.h"
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

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;

    RenderSystem(RenderSystem&&) = delete;
    RenderSystem& operator=(RenderSystem&&) = delete;

    ~RenderSystem() = default;

    bool Init();
    void Render();

    bool OnClose() const { return m_editor_ui_renderer->OnClose(); }

    void UpdateViewInfo(ViewInfo&& view_info) { m_view_window->UpdateViewInfo(std::move(view_info)); }
    MeshRenderMaterialID RegisterMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material);
    void UnregisterMeshRenderMaterial(MeshRenderMaterialID id);
    void SetSkyboxRenderMaterial(std::shared_ptr<SkyboxRenderMaterial> skybox_render_material);

    void OnSelectedSceneObjectChange() const { m_editor_ui_renderer->OnSelectedSceneObjectChange(); }

    int GetRenderTargetID() const { return m_view_window->GetRenderTargetID(); }

private:
    RenderSystem() = default;

private:
    std::unique_ptr<EditorUIRenderer> m_editor_ui_renderer;
    std::unique_ptr<ViewWindow> m_view_window;
};
} // namespace Aurora