#pragma once
// std include
#include <array>
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/RenderPass.h"
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"
#include "Core/Render/RenderGraph/RenderGraphNode.h"

namespace Aurora
{

class ShaderProgram;

class TonemapPass : public RenderPass
{
public:
    TonemapPass() = default;
    ~TonemapPass() = default;

    bool Init() override;

    RenderGraphTextureHandle Render(RenderGraphBuilder& graph_builder,
                                    RenderGraphTextureHandle scene_color,
                                    RenderGraphTextureHandle output,
                                    const std::array<int, 2>& viewport_size);

private:
    std::unique_ptr<ShaderProgram> m_shader_program;
};
} // namespace Aurora
