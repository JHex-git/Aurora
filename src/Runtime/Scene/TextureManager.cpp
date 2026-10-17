// std include
#include <cassert>

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"

// Aurora include
#include "Runtime/Scene/TextureManager.h"

namespace Aurora
{

namespace
{

// Creates a fallback texture and registers it in the manager's table; returns its ID (0 on failure).
TextureID RegisterDummyTexture(std::unordered_map<TextureID, SurfaceTexture>& textures,
    const unsigned char (&pixel)[4], const char* surface_type)
{
    const TextureDescription k_dummy_texture_description = {.width = 1,
                                                            .height = 1,
                                                            .internal_format = GL_RGBA,
                                                            .format = GL_RGBA,
                                                            .type = GL_UNSIGNED_BYTE,
                                                            .wrap_s = TextureWrapMode::ClampToEdge,
                                                            .wrap_t = TextureWrapMode::ClampToEdge};
    auto texture = TextureFactory::Create(k_dummy_texture_description, pixel);
    if (!texture)
    {
        spdlog::error("Failed to create dummy texture of type '{}'", surface_type);
        return 0;
    }
    const TextureID id = texture->GetID();
    textures.emplace(id, SurfaceTexture(std::move(*texture), surface_type));
    return id;
}
} // namespace

SurfaceTexture& TextureManager::GetDummyWhiteTexture()
{
    static const unsigned char white_pixel[] = { 255, 255, 255, 255 };
    static TextureID dummy_white_id = 0;
    auto it = m_surface_textures.find(dummy_white_id);
    if (dummy_white_id == 0 || it == m_surface_textures.end())
    {
        dummy_white_id = RegisterDummyTexture(m_surface_textures, white_pixel, "Diffuse");
        it = m_surface_textures.find(dummy_white_id);
    }
    assert(it != m_surface_textures.end());
    return it->second;
}

SurfaceTexture& TextureManager::GetDummyBlackTexture()
{
    static const unsigned char black_pixel[] = { 0, 0, 0, 255 };
    static TextureID dummy_black_id = 0;
    auto it = m_surface_textures.find(dummy_black_id);
    if (dummy_black_id == 0 || it == m_surface_textures.end())
    {
        dummy_black_id = RegisterDummyTexture(m_surface_textures, black_pixel, "Black");
        it = m_surface_textures.find(dummy_black_id);
    }
    assert(it != m_surface_textures.end());
    return it->second;
}

SurfaceTexture& TextureManager::GetDummyNormalTexture()
{
    static const unsigned char normal_pixel[] = { 128, 128, 255, 255 };
    static TextureID dummy_normal_id = 0;
    auto it = m_surface_textures.find(dummy_normal_id);
    if (dummy_normal_id == 0 || it == m_surface_textures.end())
    {
        dummy_normal_id = RegisterDummyTexture(m_surface_textures, normal_pixel, "Normal");
        it = m_surface_textures.find(dummy_normal_id);
    }
    assert(it != m_surface_textures.end());
    return it->second;
}

SurfaceTexture& TextureManager::GetTexture(TextureID id)
{
    return m_surface_textures.find(id)->second;
}

} // namespace Aurora
