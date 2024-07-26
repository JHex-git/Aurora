// std include

// thirdparty include
#include "thirdparty/stb/stb_image.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "glWrapper/Texture.h"
#include "Utility/FileSystem.h"

namespace Aurora
{

std::optional<Texture> TextureBuilder::MakeTexture2D(const std::string& path)
{
    if (m_gen_mipmap && (m_min_filter == TextureBuilder::FilterType::Nearest || m_min_filter == TextureBuilder::FilterType::Linear))
    {
        spdlog::error("Mipmap generation requires mipmap min filter");
        return std::nullopt;
    }

    int width = 0, height = 0, channels = 0;
    std::string full_path = FileSystem::GetFullPath(path);
    stbi_set_flip_vertically_on_load(true);
    void* data = m_require_high_precision ? stbi_loadf(full_path.c_str(), &width, &height, &channels, 0) : (void *)stbi_load(full_path.c_str(), &width, &height, &channels, 0);
    if (data)
    {
        GLenum format;
        switch (channels)
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
            spdlog::warn("Unsupported texture format, use RGB instead");
            format = GL_RGB;
            break;
        }

        Texture texture(Texture::TextureType::Texture2D);
        texture.Bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, m_require_high_precision ? GL_FLOAT : GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(m_min_filter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(m_mag_filter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(m_wrap_s));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(m_wrap_t));
        if (m_gen_mipmap) glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        spdlog::info("Texture2D {} loaded", path);
        return texture;
    }
    else
    {
        spdlog::error("Failed to load texture {}", path);
        return std::nullopt;
    }
}

std::optional<Texture> TextureBuilder::MakeTextureCubeMap(const std::array<std::string, 6>& paths)
{
    if (m_gen_mipmap && (m_min_filter == TextureBuilder::FilterType::Nearest || m_min_filter == TextureBuilder::FilterType::Linear))
    {
        spdlog::error("Mipmap generation requires mipmap min filter");
        return std::nullopt;
    }

    Texture texture(Texture::TextureType::Cubemap);
    texture.Bind();
    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < paths.size(); ++i)
    {
        std::string full_path = FileSystem::GetFullPath(paths[i]);
        int width = 0, height = 0, channels = 0;
        void* data = m_require_high_precision ? stbi_loadf(full_path.c_str(), &width, &height, &channels, 0) : (void *)stbi_load(full_path.c_str(), &width, &height, &channels, 0);
        if (data)
        {
            GLenum format;
            switch (channels)
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

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, format, m_require_high_precision ? GL_FLOAT : GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(m_min_filter));
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(m_mag_filter));
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, static_cast<GLint>(TextureBuilder::WrapType::ClampToEdge));
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, static_cast<GLint>(TextureBuilder::WrapType::ClampToEdge));
            stbi_image_free(data);
            spdlog::info("Texture {} loaded", paths[i]);
        }
        else
        {
            spdlog::error("Failed to load texture {}", paths[i]);
            return std::nullopt;
        }
    }
    if (m_gen_mipmap) glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    return std::move(texture);
}

Texture::Texture(TextureType texture_type): m_type(texture_type), m_unit(-1)
{
    glGenTextures(1, &m_textureID);
    spdlog::info("Texture {} created", m_textureID);
}

Texture::~Texture()
{
    if (m_textureID != 0)
    {
        spdlog::info("Texture {} deleted", m_textureID);
        glDeleteTextures(1, &m_textureID);
    }
}

Texture::Texture(Texture&& other)
{
    m_textureID = other.m_textureID;
    m_type = other.m_type;
    m_unit = other.m_unit;
    other.m_textureID = 0;
}

Texture& Texture::operator=(Texture&& other)
{
    if (this != &other)
    {
        glDeleteTextures(1, &m_textureID);
        m_textureID = other.m_textureID;
        m_type = other.m_type;
        m_unit = other.m_unit;
        other.m_textureID = 0;
    }
    return *this;
}

void Texture::Bind(unsigned int unit/* = 0 */)
{
    m_unit = unit;
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(static_cast<GLenum>(m_type), m_textureID);
}

void Texture::Unbind()
{
    glActiveTexture(GL_TEXTURE0 + m_unit);
    glBindTexture(static_cast<GLenum>(m_type), 0);
    m_unit = -1;
}
} // namespace Aurora
