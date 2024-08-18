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

    MeshRenderMaterialID RegisterMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material);
    void UnregisterMeshRenderMaterial(MeshRenderMaterialID id);

private:
    std::unique_ptr<ShaderProgram> m_tex_shader_program;
    std::unique_ptr<ShaderProgram> m_no_tex_shader_program;
    std::unique_ptr<ShaderProgram> m_shadow_map_shader_program;
    std::unique_ptr<ShaderProgram> m_composite_shader_program;
    std::unordered_map<MeshRenderMaterialID, std::weak_ptr<MeshRenderMaterial>> m_mesh_render_materials;

    std::shared_ptr<FrameBufferObject> m_shadow_map_fbo;
    std::shared_ptr<FrameBufferObject> m_shading_fbo;
};
} // namespace Aurora