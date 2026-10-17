// std include
#include <cassert>
// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/RenderGraph/RenderGraphContext.h"
#include "Core/Render/RenderResourcePool.h"
#include "glWrapper/FrameBufferObject.h"

namespace Aurora
{

RenderGraphContext::RenderGraphContext(ContextState& context_state,
                                       RenderResourcePool& resource_pool,
                                       const std::vector<std::unique_ptr<RenderGraphResource>>& resources,
                                       const std::array<int, 2>& viewport_size)
    : m_context_state(context_state),
      m_resource_pool(resource_pool),
      m_resources(resources),
      m_viewport_size(viewport_size)
{
}

Texture* RenderGraphContext::TryGetTexture(RenderGraphTextureHandle handle) const
{
    if (!handle.IsValid() || handle.id >= m_resources.size()) return nullptr;

    const auto& resource = m_resources[handle.id];
    if (resource == nullptr) return nullptr;
    if (resource->GetType() != RenderGraphResource::Type::Texture) return nullptr;

    return static_cast<const RenderGraphTexture*>(resource.get())->GetResolvedTexture();
}

Texture& RenderGraphContext::GetTexture(RenderGraphTextureHandle handle) const
{
    Texture* texture = TryGetTexture(handle);
    if (texture == nullptr)
    {
        spdlog::error("RenderGraphContext::GetTexture: handle {} is not a resolved texture", handle.id);
        assert(false && "RenderGraphContext::GetTexture: resource is missing or not resolved");
    }
    return *texture;
}

FrameBufferObject& RenderGraphContext::GetFrameBuffer(const std::vector<RenderGraphTextureHandle>& colors,
                                                      RenderGraphTextureHandle depth) const
{
    return GetFrameBuffer(colors, depth, m_viewport_size);
}

FrameBufferObject& RenderGraphContext::GetFrameBuffer(const std::vector<RenderGraphTextureHandle>& colors,
                                                      RenderGraphTextureHandle depth,
                                                      const std::array<int, 2>& size) const
{
    FrameBufferKey key;
    key.colors.reserve(colors.size());
    for (const RenderGraphTextureHandle handle : colors)
    {
        key.colors.push_back(&GetTexture(handle));
    }
    if (depth.IsValid())
    {
        key.depth = &GetTexture(depth);
    }
    key.width = size[0];
    key.height = size[1];

    FrameBufferObject* framebuffer = m_resource_pool.AcquireFramebuffer(key);
    assert(framebuffer != nullptr && "RenderGraphContext::GetFrameBuffer: failed to create the framebuffer");
    return *framebuffer;
}

} // namespace Aurora
