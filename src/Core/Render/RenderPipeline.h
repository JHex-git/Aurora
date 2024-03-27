#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/MeshPhongPass.h"

namespace Aurora
{

class RenderPipeline
{
public:
    RenderPipeline() = default;
    ~RenderPipeline() = default;

    bool Init();
    void Render();

    void AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material);

private:
    std::unique_ptr<MeshPhongPass> m_mesh_phong_pass;
};
} // namespace Aurora