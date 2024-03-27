#pragma once
// std include
#include <memory>
#include <array>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/MeshPhongPass.h"
#include "Core/Render/RenderPipeline.h"

namespace Aurora
{

struct ViewInfo
{
    std::array<int, 2> m_size;
    std::array<int, 2> m_position; // left bottom corner
};

class ViewWindow
{
public:
    ViewWindow() = default;
    ~ViewWindow() = default;

    bool Init();
    void Render();

    void AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material) { m_render_pipeline->AddMeshRenderMaterial(mesh_render_material); }
    void UpdateViewInfo(ViewInfo&& view_info) { m_view_info = std::move(view_info); }

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