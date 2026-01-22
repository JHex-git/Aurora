#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "glWrapper/Shader.h"

namespace Aurora
{

class GizmosPass : public RenderPass
{
public:
    GizmosPass() = default;
    virtual ~GizmosPass() = default;

    bool Init(const std::array<int, 2>& viewport_size) override;
    void Render(ContextState& context_state) override;

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