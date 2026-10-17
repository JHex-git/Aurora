#pragma once
// std include
#include <unordered_map>
#include <memory>
// thirdparty include
// Aurora include
#include "glWrapper/Texture.h"
#include "glWrapper/FrameBufferObject.h"

namespace Aurora
{

class RenderResourcePool
{
public:
    RenderResourcePool() = default;
    ~RenderResourcePool() = default;

    RenderResourcePool(const RenderResourcePool&) = delete;
    RenderResourcePool& operator=(const RenderResourcePool&) = delete;

    RenderResourcePool(RenderResourcePool&&) = delete;
    RenderResourcePool& operator=(RenderResourcePool&&) = delete;

    Texture* AcquireTexture(const TextureDescription& description);
    void ReleaseTexture(Texture* texture);
    // void TrimUnusedTextures(); TODO:

    FrameBufferObject* AcquireFramebuffer(const FrameBufferKey& key);

private:
    struct AcquiredTexture
    {
        TextureDescription description;
        std::unique_ptr<Texture> texture;
    };
    std::unordered_map<TextureDescription, std::vector<std::unique_ptr<Texture>>> m_free_textures;
    std::unordered_map<Texture*, AcquiredTexture> m_acquired_textures;

    std::unordered_map<FrameBufferKey, std::unique_ptr<FrameBufferObject>> m_framebuffers;
};
} // namespace Aurora
