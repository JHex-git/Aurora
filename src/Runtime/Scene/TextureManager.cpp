// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/TextureManager.h"

namespace Aurora
{

SurfaceTexture& TextureManager::GetDummyWhiteTexture()
{
    static TextureID dummy_white_id = 0;
    auto it = m_surface_textures.find(dummy_white_id);
    if (dummy_white_id == 0 || it == m_surface_textures.end())
    {
        Texture texture(Texture::Type::Texture2D);
        texture.Bind();
        unsigned char white_pixel[] = { 255, 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP_TO_EDGE));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP_TO_EDGE));
        dummy_white_id = texture.GetID();
        m_surface_textures.emplace(dummy_white_id, SurfaceTexture(std::move(texture), "Diffuse"));
        it = m_surface_textures.find(dummy_white_id);
    }
    return it->second;
}

SurfaceTexture& TextureManager::GetDummyBlackTexture()
{
    static TextureID dummy_black_id = 0;
    auto it = m_surface_textures.find(dummy_black_id);
    if (dummy_black_id == 0 || it == m_surface_textures.end())
    {
        Texture texture(Texture::Type::Texture2D);
        texture.Bind();
        unsigned char black_pixel[] = { 0, 0, 0, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, black_pixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP_TO_EDGE));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP_TO_EDGE));
        dummy_black_id = texture.GetID();
        m_surface_textures.emplace(dummy_black_id, SurfaceTexture(std::move(texture), "Black"));
        it = m_surface_textures.find(dummy_black_id);
    }
    return it->second;
}

SurfaceTexture& TextureManager::GetDummyNormalTexture()
{
    static TextureID dummy_normal_id = 0;
    auto it = m_surface_textures.find(dummy_normal_id);
    if (dummy_normal_id == 0 || it == m_surface_textures.end())
    {
        Texture texture(Texture::Type::Texture2D);
        texture.Bind();
        unsigned char normal_pixel[] = { 128, 128, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, normal_pixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP_TO_EDGE));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP_TO_EDGE));
        dummy_normal_id = texture.GetID();
        m_surface_textures.emplace(dummy_normal_id, SurfaceTexture(std::move(texture), "Normal"));
        it = m_surface_textures.find(dummy_normal_id);
    }
    return it->second;
}

SurfaceTexture& TextureManager::GetTexture(TextureID id)
{
    return m_surface_textures.find(id)->second;
}

} // namespace Aurora
