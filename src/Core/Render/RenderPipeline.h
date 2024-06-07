#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/MeshPhongPass.h"
#include "Core/Render/Pass/MeshOutlinePass.h"

namespace Aurora
{

class RenderPipeline
{
public:
    RenderPipeline() = default;
    ~RenderPipeline() = default;

    bool Init();
    void Render(const std::array<int, 2>& viewport_size);

    void AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material);

private:
    std::unique_ptr<MeshPhongPass> m_mesh_phong_pass;
    std::unique_ptr<MeshOutlinePass> m_mesh_outline_pass;
};
} // namespace Aurora