#pragma once
// std include
#include <array>
#include <initializer_list>
#include <memory>
#include <vector>
// Aurora include
#include "Core/Render/RenderGraph/RenderGraphTexture.h"
#include "glWrapper/ContextState.h"

namespace Aurora
{
class FrameBufferObject;
class RenderResourcePool;

class RenderGraphContext
{
public:
    RenderGraphContext(ContextState& context_state,
                       RenderResourcePool& resource_pool,
                       const std::vector<std::unique_ptr<RenderGraphResource>>& resources,
                       const std::array<int, 2>& viewport_size);

    Texture& GetTexture(RenderGraphTextureHandle handle) const;

    Texture* TryGetTexture(RenderGraphTextureHandle handle) const;

    FrameBufferObject& GetFrameBuffer(const std::vector<RenderGraphTextureHandle>& colors,
                                      RenderGraphTextureHandle depth = {}) const;

    FrameBufferObject& GetFrameBuffer(const std::vector<RenderGraphTextureHandle>& colors,
                                      RenderGraphTextureHandle depth,
                                      const std::array<int, 2>& size) const;

    void ApplyRenderState(const RenderState& state) const { m_context_state.ApplyRenderState(state); }
    ContextState& GetContextState() const { return m_context_state; }

    int GetWidth() const { return m_viewport_size[0]; }
    int GetHeight() const { return m_viewport_size[1]; }
    const std::array<int, 2>& GetViewportSize() const { return m_viewport_size; }

private:
    ContextState& m_context_state;
    RenderResourcePool& m_resource_pool;
    const std::vector<std::unique_ptr<RenderGraphResource>>& m_resources;
    std::array<int, 2> m_viewport_size;
};
} // namespace Aurora
