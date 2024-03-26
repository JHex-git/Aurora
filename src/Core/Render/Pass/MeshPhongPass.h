#pragma once
// std include

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Material/MeshRenderMaterial.h"

namespace Aurora
{

class MeshPhongPass : public RenderPass
{
public:
    MeshPhongPass() = default;
    ~MeshPhongPass() = default;

    bool Init(GLFWwindow* window) override;
    void Render() override;

    void AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material) { m_mesh_render_materials.push_back(mesh_render_material); }

private:
    std::unique_ptr<ShaderProgram> m_shader_program;
    std::vector<std::shared_ptr<MeshRenderMaterial>> m_mesh_render_materials;
};
} // namespace Aurora