// std include
#include <algorithm>

// thirdparty include
#include "thirdparty/stb/stb_image.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "glWrapper/Texture.h"
#include "Utility/FileSystem.h"

namespace Aurora
{

namespace
{
// CubemapArray stores 6 * layers GL layers; other targets use layers directly.
GLsizei GetStorageLayers(const TextureDescription& description)
{
    switch (description.texture_type)
    {
    case TextureType::Cubemap:
        return 6;
    case TextureType::CubemapArray:
        return std::max(description.layers, 1) * 6;
    case TextureType::Texture2DArray:
        return std::max(description.layers, 1);
    case TextureType::Texture2D:
    default:
        return 1;
    }
}

bool SupportsWrapR(TextureType type)
{
    return type == TextureType::Cubemap || type == TextureType::CubemapArray || type == TextureType::Texture2DArray;
}

void ApplyTextureParameters(const TextureDescription& description)
{
    const GLenum target = static_cast<GLenum>(description.texture_type);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(description.min_filter));
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(description.mag_filter));
    glTexParameteri(target, GL_TEXTURE_WRAP_S, static_cast<GLint>(description.wrap_s));
    glTexParameteri(target, GL_TEXTURE_WRAP_T, static_cast<GLint>(description.wrap_t));
    if (SupportsWrapR(description.texture_type))
    {
        glTexParameteri(target, GL_TEXTURE_WRAP_R, static_cast<GLint>(description.wrap_r));
    }
}

GLenum ChannelCountToPixelFormat(int channels)
{
    switch (channels)
    {
    case 1:
        return GL_RED;
    case 2:
        return GL_RG;
    case 3:
        return GL_RGB;
    case 4:
        return GL_RGBA;
    default:
        spdlog::warn("Unsupported channel count {}, falling back to GL_RGB", channels);
        return GL_RGB;
    }
}

// Only 3/4 channel images get an sRGB internal format, single/two channel still use GL_RGB.
GLint ResolveFileInternalFormat(int channels, const TextureLoadOptions& options)
{
    if (options.srgb && !options.high_precision)
    {
        if (channels == 4) return GL_SRGB_ALPHA;
        if (channels == 3) return GL_SRGB;
    }
    return GL_RGB;
}

bool ValidateMipmapRequest(const TextureLoadOptions& options, const char* loader_name, const std::string& path)
{
    if (options.gen_mipmap && (options.min_filter == TextureFilterMode::Nearest || options.min_filter == TextureFilterMode::Linear))
    {
        spdlog::error("{}: mipmap generation requires a mipmap min filter ({})", loader_name, path);
        return false;
    }
    return true;
}

void* LoadImageData(const std::string& full_path, bool high_precision, bool flip_vertically, int& width, int& height, int& channels)
{
    stbi_set_flip_vertically_on_load(flip_vertically);
    return high_precision
        ? static_cast<void*>(stbi_loadf(full_path.c_str(), &width, &height, &channels, 0))
        : static_cast<void*>(stbi_load(full_path.c_str(), &width, &height, &channels, 0));
}
} // namespace

std::optional<Texture> TextureFactory::Create(const TextureDescription& description, const void* initial_data)
{
    if (description.width <= 0 || description.height <= 0)
    {
        spdlog::error("TextureFactory::Create: invalid size {}x{}", description.width, description.height);
        return std::nullopt;
    }

    Texture texture(description.texture_type);
    texture.m_description = description;
    texture.Bind();

    const GLenum target = static_cast<GLenum>(description.texture_type);
    const GLsizei layers = GetStorageLayers(description);

    if (description.texture_type == TextureType::Texture2D)
    {
        glTexImage2D(target, 0, description.internal_format, description.width, description.height, 0,
                     description.format, description.type, initial_data);
    }
    else if (description.texture_type == TextureType::Cubemap)
    {
        for (unsigned int face = 0; face < 6; ++face)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, description.internal_format,
                         description.width, description.height, 0, description.format, description.type, initial_data);
        }
    }
    else
    {
        glTexImage3D(target, 0, description.internal_format, description.width, description.height, layers, 0,
                     description.format, description.type, initial_data);
    }

    ApplyTextureParameters(description);

    if (description.gen_mipmap) glGenerateMipmap(target);

    return texture;
}

std::optional<Texture> TextureLoader::Load2D(const std::string& path, const TextureLoadOptions& options)
{
    if (!ValidateMipmapRequest(options, "TextureLoader::Load2D", path))
        return std::nullopt;

    int width = 0, height = 0, channels = 0;
    const std::string full_path = FileSystem::GetFullPath(path);
    void* data = LoadImageData(full_path, options.high_precision, options.flip_vertically, width, height, channels);
    if (!data)
    {
        spdlog::error("Failed to load texture {}", path);
        return std::nullopt;
    }

    const TextureDescription description{
        .width = width,
        .height = height,
        .texture_type = TextureType::Texture2D,
        .internal_format = ResolveFileInternalFormat(channels, options),
        .format = ChannelCountToPixelFormat(channels),
        .type = static_cast<GLenum>(options.high_precision ? GL_FLOAT : GL_UNSIGNED_BYTE),
        .wrap_s = options.wrap_s,
        .wrap_t = options.wrap_t,
        .wrap_r = options.wrap_r,
        .min_filter = options.min_filter,
        .mag_filter = options.mag_filter,
        .gen_mipmap = options.gen_mipmap};

    auto texture = TextureFactory::Create(description, data);
    stbi_image_free(data);

    if (!texture)
    {
        spdlog::error("Failed to create texture from {}", path);
        return std::nullopt;
    }

    spdlog::info("Texture2D {} loaded", path);
    return texture;
}

std::optional<Texture> TextureLoader::LoadCubemap(const std::array<std::string, 6>& paths, const TextureLoadOptions& options)
{
    if (!ValidateMipmapRequest(options, "TextureLoader::LoadCubemap", paths[0]))
        return std::nullopt;

    Texture texture(TextureType::Cubemap);
    texture.Bind();

    const GLenum pixel_type = options.high_precision ? GL_FLOAT : GL_UNSIGNED_BYTE;
    int loaded_width = 0;
    int loaded_height = 0;
    GLint loaded_internal_format = GL_RGB;
    GLenum loaded_format = GL_RGB;
    for (size_t i = 0; i < paths.size(); ++i)
    {
        int width = 0, height = 0, channels = 0;
        const std::string full_path = FileSystem::GetFullPath(paths[i]);
        void* data = LoadImageData(full_path, options.high_precision, options.flip_vertically, width, height, channels);
        if (!data)
        {
            spdlog::error("Failed to load texture {}", paths[i]);
            return std::nullopt;
        }

        loaded_width = width;
        loaded_height = height;
        loaded_internal_format = ResolveFileInternalFormat(channels, options);
        loaded_format = ChannelCountToPixelFormat(channels);
        glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0,
                     loaded_internal_format, width, height, 0,
                     loaded_format, pixel_type, data);
        stbi_image_free(data);
        spdlog::info("Texture {} loaded", paths[i]);
    }

    const TextureDescription description{
        .width = loaded_width,
        .height = loaded_height,
        .texture_type = TextureType::Cubemap,
        .internal_format = loaded_internal_format,
        .format = loaded_format,
        .type = pixel_type,
        .wrap_s = options.wrap_s,
        .wrap_t = options.wrap_t,
        .wrap_r = options.wrap_r,
        .min_filter = options.min_filter,
        .mag_filter = options.mag_filter,
        .gen_mipmap = options.gen_mipmap};

    texture.m_description = description;
    ApplyTextureParameters(description);

    if (options.gen_mipmap) glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return texture;
}

Texture::Texture(Type texture_type): m_type(texture_type), m_unit(-1)
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
    m_description = other.m_description;
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
        m_description = other.m_description;
        other.m_textureID = 0;
    }
    return *this;
}

bool Texture::HasStencil() const
{
    switch (m_description.internal_format)
    {
    case GL_DEPTH_STENCIL:
    case GL_DEPTH24_STENCIL8:
    case GL_DEPTH32F_STENCIL8:
        return true;
    default:
        return m_description.format == GL_DEPTH_STENCIL;
    }
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
