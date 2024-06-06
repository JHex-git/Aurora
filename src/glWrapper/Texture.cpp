// std include

// thirdparty include
#include "thirdparty/stb/stb_image.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "glWrapper/Texture.h"
#include "Utility/FileSystem.h"

namespace Aurora
{

Texture::Texture(): m_isLoaded(false)
{
    // TODO:
    glGenTextures(1, &m_textureID);
    spdlog::info("Texture {} created", m_textureID);
}

Texture::~Texture()
{
    // TODO:
    spdlog::info("Texture {} deleted", m_textureID);
    glDeleteTextures(1, &m_textureID);
}

Texture::Texture(Texture&& other)
{
    m_textureID = other.m_textureID;
    m_unit = other.m_unit;
    m_width = other.m_width;
    m_height = other.m_height;
    m_channels = other.m_channels;
    m_isLoaded = other.m_isLoaded;
    other.m_textureID = 0;
}

Texture& Texture::operator=(Texture&& other)
{
    if (this != &other)
    {
        glDeleteTextures(1, &m_textureID);
        m_textureID = other.m_textureID;
        m_unit = other.m_unit;
        m_width = other.m_width;
        m_height = other.m_height;
        m_channels = other.m_channels;
        m_isLoaded = other.m_isLoaded;
        other.m_textureID = 0;
    }
    return *this;
}

bool Texture::Load(const std::string& path, WrapType wrap_s, WrapType wrap_t, FilterType min_filter, FilterType mag_filter, bool gen_mipmap, bool require_high_precision)
{
    if (!(gen_mipmap || (min_filter == FilterType::Nearest || min_filter == FilterType::Linear)))
    {
        spdlog::error("Mipmap generation requires mipmap min filter");
        return m_isLoaded = false;
    }
    std::string full_path = FileSystem::GetFullPath(path);
    void* data = require_high_precision ? stbi_loadf(full_path.c_str(), &m_width, &m_height, &m_channels, 0) : (void *)stbi_load(full_path.c_str(), &m_width, &m_height, &m_channels, 0);
    if (data)
    {
        GLenum format;
        switch (m_channels)
        {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            spdlog::error("Unsupported texture format");
            format = GL_RGB;
            break;
        }

        Bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, format, require_high_precision ? GL_FLOAT : GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(min_filter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(mag_filter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(wrap_s));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(wrap_t));
        if (gen_mipmap) glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        spdlog::info("Texture {} loaded", path);
        return m_isLoaded = true;
    }
    else
    {
        spdlog::error("Failed to load texture {}", path);
        return m_isLoaded = false;
    }
}

void Texture::Bind(unsigned int unit/* = 0 */)
{
    m_unit = unit;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::Unbind()
{
    glActiveTexture(GL_TEXTURE0 + m_unit);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_unit = -1;
}

} // namespace Aurora
