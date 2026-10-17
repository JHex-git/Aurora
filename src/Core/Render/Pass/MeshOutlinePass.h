#pragma once
// std include
#include <array>
#include <memory>

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "Core/Render/RenderGraph/RenderGraphTexture.h"
#include "glWrapper/Shader.h"

namespace Aurora
{

class RenderGraphBuilder;

class MeshOutlinePass : public RenderPass
{
public:
    MeshOutlinePass() = default;
    ~MeshOutlinePass() = default;

    bool Init() override;

    RenderGraphTextureHandle Render(RenderGraphBuilder& graph_builder,
                                    RenderGraphTextureHandle color,
                                    RenderGraphTextureHandle depth,
                                    RenderGraphTextureHandle& output_depth,
                                    const std::array<int, 2>& viewport_size);

    void SetMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material) { m_selected_mesh_render_material = mesh_render_material; }

private:
    std::unique_ptr<ShaderProgram> m_mesh_stencil_shader_program;
    std::unique_ptr<ShaderProgram> m_outline_shader_program;
    std::weak_ptr<MeshRenderMaterial> m_selected_mesh_render_material;
};
} // namespace Aurora