#pragma once
// std include
#include <string>
#include <utility>
// thirdparty include
// Aurora include
#include "Core/Render/RenderGraph/RenderGraphNode.h"
#include "glWrapper/Texture.h"

namespace Aurora
{
class RenderGraphTexture : public RenderGraphResource
{
    friend class RenderGraphBuilder;
public:
    RenderGraphTexture(uint32_t id, std::string name, TextureDescription&& description, bool is_external = false)
        : RenderGraphResource(id, is_external), m_name(std::move(name)), m_description(std::move(description)) {}

    const std::string& GetName() const { return m_name; }
    const TextureDescription& GetDescription() const { return m_description; }

    Texture* GetResolvedTexture() const { return m_resolved; }
    void Resolve(Texture* texture) { m_resolved = texture; }
private:
    std::string m_name;
    TextureDescription m_description;
    Texture* m_resolved = nullptr;
};

using RenderGraphTextureHandle = RenderGraphResourceHandle;
} // namespace Aurora
