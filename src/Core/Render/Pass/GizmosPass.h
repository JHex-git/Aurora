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

class GizmosPass : public RenderPass
{
public:
    GizmosPass() = default;
    virtual ~GizmosPass() = default;

    bool Init() override;

    RenderGraphTextureHandle Render(RenderGraphBuilder& graph_builder,
                                    RenderGraphTextureHandle color,
                                    RenderGraphTextureHandle depth);

    void SetSelectedTransform(std::shared_ptr<Transform> select_transform) { m_selected_transform = select_transform; }

private:
    std::unique_ptr<ShaderProgram> m_shader_program;
    std::weak_ptr<Transform> m_selected_transform;

    std::shared_ptr<MeshRenderMaterial> m_translation_handle_material;
    std::shared_ptr<MeshRenderMaterial> m_rotation_handle_material;
    std::shared_ptr<MeshRenderMaterial> m_scale_handle_material;
    std::shared_ptr<MeshRenderMaterial> m_current_handle_material;
};
} // namespace Aurora