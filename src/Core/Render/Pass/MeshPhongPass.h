#pragma once
// std include

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "glWrapper/Shader.h"

namespace Aurora
{

class MeshPhongPass : public RenderPass
{
public:
    MeshPhongPass() = default;
    ~MeshPhongPass() = default;

    bool Init(const std::array<int, 2>& viewport_size) override;
    void Render() override;

    void AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material) { m_mesh_render_materials.push_back(mesh_render_material); }

private:
    std::unique_ptr<ShaderProgram> m_tex_shader_program;
    std::unique_ptr<ShaderProgram> m_no_tex_shader_program;
    std::unique_ptr<ShaderProgram> m_shadow_map_shader_program;
    std::vector<std::shared_ptr<MeshRenderMaterial>> m_mesh_render_materials;

    std::shared_ptr<FrameBufferObject> m_shadow_map_fbo;
};
} // namespace Aurora