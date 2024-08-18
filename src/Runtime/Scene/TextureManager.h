#pragma once
// std include
#include <unordered_map>
// thirdparty include

// Aurora include
#include "glWrapper/Texture.h"

namespace Aurora
{

class TextureManager
{
    friend class Mesh;
public:
    static TextureManager& GetInstance()
    {
        static TextureManager instance;
        return instance;
    }

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

    SurfaceTexture& GetDummyWhiteTexture();

    void Reset() { m_surface_textures.clear(); }
    
    // Note: Make sure the texture exists before calling this function, otherwise it will crash.
    SurfaceTexture& GetTexture(TextureID id);
private:
    TextureManager() = default;
    ~TextureManager() = default;

    std::unordered_map<TextureID, SurfaceTexture> m_surface_textures;
};
} // namespace Aurora