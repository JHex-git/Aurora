#pragma once
// std include
#include <memory>
#include <array>
// thirdparty include

// Aurora include
#include "Core/Render/RenderPipeline.h"

namespace Aurora
{

struct ViewInfo
{
    std::array<int, 2> size;
    std::array<int, 2> position; // left bottom corner
};

class ViewWindow
{
public:
    ViewWindow() = default;
    ~ViewWindow() = default;

    bool Init();
    void Render();

    void SetSkyboxRenderMaterial(std::shared_ptr<SkyboxRenderMaterial> skybox_render_material) { m_render_pipeline->SetSkyboxRenderMaterial(skybox_render_material); }
    void UpdateViewInfo(ViewInfo&& view_info) { m_view_info = std::move(view_info); }
    std::array<int, 2> GetSize() const { return m_view_info.size; }

    int GetRenderTargetID() const { return m_render_pipeline->GetRenderTargetID(); }

private:
    void OnMousePosCallback(double xpos, double ypos);
    void OnMouseButtonCallback(int button, int action, int mods);
    void OnScrollCallback(double xoffset, double yoffset);
    void OnKeyCallback(int key, int scancode, int action, int mods);

private:
    std::unique_ptr<RenderPipeline> m_render_pipeline;
    ViewInfo m_view_info;

    double m_last_x = 0.0;
    double m_last_y = 0.0;
};
} // namespace Aurora