// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/TextureManager.h"

namespace Aurora
{

SurfaceTexture& TextureManager::GetDummyWhiteTexture()
{
    static unsigned int dummy_white_id = [&]() {
        Texture texture(Texture::Type::Texture2D);
        texture.Bind();
        unsigned char white_pixel[] = { 255, 255, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP_TO_EDGE));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP_TO_EDGE));
        auto id = texture.GetID();
        m_surface_textures.emplace(id, SurfaceTexture(std::move(texture), "Diffuse"));
        return id;
    }();
    return m_surface_textures.find(dummy_white_id)->second;
}

SurfaceTexture& TextureManager::GetTexture(TextureID id)
{
    return m_surface_textures.find(id)->second;
}

} // namespace Aurora