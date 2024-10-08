#pragma once
// std include

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Material/SkyboxRenderMaterial.h"
#include "glWrapper/Shader.h"

namespace Aurora
{

class SkyboxPass : public RenderPass
{
public:
    SkyboxPass() = default;
    ~SkyboxPass() = default;

    bool Init(const std::array<int, 2>& viewport_size) override;
    void Render() override;

    void SetSkyboxRenderMaterial(std::shared_ptr<SkyboxRenderMaterial> skybox_render_material) { m_skybox_render_material = skybox_render_material; }

private:
    std::unique_ptr<ShaderProgram> m_shader_program;
    std::weak_ptr<SkyboxRenderMaterial> m_skybox_render_material;
};
} // namespace Aurora