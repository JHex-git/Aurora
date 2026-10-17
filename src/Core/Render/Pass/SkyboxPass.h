#pragma once
// std include
#include <array>
#include <memory>

// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/Material/SkyboxRenderMaterial.h"
#include "Core/Render/RenderGraph/RenderGraphTexture.h"
#include "glWrapper/Shader.h"

namespace Aurora
{

class RenderGraphBuilder;

class SkyboxPass : public RenderPass
{
public:
    SkyboxPass() = default;
    ~SkyboxPass() = default;

    bool Init() override;

    RenderGraphTextureHandle Render(RenderGraphBuilder& graph_builder,
                                    RenderGraphTextureHandle color,
                                    RenderGraphTextureHandle depth);

    void SetSkyboxRenderMaterial(std::shared_ptr<SkyboxRenderMaterial> skybox_render_material) { m_skybox_render_material = skybox_render_material; }

private:
    std::unique_ptr<ShaderProgram> m_shader_program;
    std::weak_ptr<SkyboxRenderMaterial> m_skybox_render_material;
};
} // namespace Aurora