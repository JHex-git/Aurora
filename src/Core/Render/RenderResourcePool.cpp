#include "Core/Render/RenderResourcePool.h"
#include "glWrapper/Texture.h"

namespace Aurora
{

Texture* RenderResourcePool::AcquireTexture(const TextureDescription & description)
{
    std::unique_ptr<Texture> texture;
    if (auto it = m_free_textures.find(description); it != m_free_textures.end() && !it->second.empty())
    {
        texture = std::move(it->second.back());
        it->second.pop_back();
    }
    else if (auto created = TextureFactory::Create(description))
    {
        texture = std::make_unique<Texture>(std::move(*created));
    }

    if (!texture) return nullptr;
    Texture* raw_ptr = texture.get();
    m_acquired_textures[raw_ptr] = { description, std::move(texture) };
    return raw_ptr;
}

void RenderResourcePool::ReleaseTexture(Texture* texture)
{
    if (texture == nullptr) return;
    if (auto it = m_acquired_textures.find(texture); it != m_acquired_textures.end())
    {
        m_free_textures[it->second.description].push_back(std::move(it->second.texture));
        m_acquired_textures.erase(it);
    }
}

FrameBufferObject* RenderResourcePool::AcquireFramebuffer(const FrameBufferKey & key)
{
    if (auto it = m_framebuffers.find(key); it != m_framebuffers.end())
    {
        return it->second.get();
    }
    else
    {
        auto fbo = FrameBufferObject::Create(key.width, key.height, key.colors, key.depth);
        if (!fbo) return nullptr;
        auto fbo_ptr = std::make_unique<FrameBufferObject>(std::move(*fbo));
        FrameBufferObject* fbo_raw_ptr = fbo_ptr.get();
        m_framebuffers[key] = std::move(fbo_ptr);
        return fbo_raw_ptr;
    }
}

// void RenderResourcePool::TrimUnusedTextures()
// {
// }



} // namespace Aurora
