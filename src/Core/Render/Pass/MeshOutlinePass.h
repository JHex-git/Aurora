#pragma once
// std include

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Material/MeshRenderMaterial.h"

namespace Aurora
{

class MeshOutlinePass : public RenderPass
{
public:
    MeshOutlinePass() = default;
    ~MeshOutlinePass() = default;

    bool Init() override;
    void Render(const std::array<int, 2>& viewport_size) override;

    void SetMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material) { m_selected_mesh_render_material = mesh_render_material; }

private:
    std::unique_ptr<ShaderProgram> m_shader_program;
    std::shared_ptr<MeshRenderMaterial> m_selected_mesh_render_material;
};
} // namespace Aurora