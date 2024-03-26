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
    std::array<int, 2> m_position;
};

class ViewWindow
{
public:
    ViewWindow() = default;
    ~ViewWindow() = default;

    bool Init(GLFWwindow* window);
    void Render();

    void AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material) { m_render_pipeline->AddMeshRenderMaterial(mesh_render_material); }
    void UpdateViewInfo(ViewInfo&& view_info) { m_view_info = std::move(view_info); }

private:
    std::unique_ptr<RenderPipeline> m_render_pipeline;
    GLFWwindow* m_window = nullptr;
    ViewInfo m_view_info;
};
} // namespace Aurora