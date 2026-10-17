#pragma once
// std include
#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>
// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
// Aurora include

namespace Aurora
{

class Texture;
using TextureID = GLuint;

enum class TextureType : GLenum
{
    Texture2D = GL_TEXTURE_2D,
    Cubemap = GL_TEXTURE_CUBE_MAP,
    Texture2DArray = GL_TEXTURE_2D_ARRAY,
    CubemapArray = GL_TEXTURE_CUBE_MAP_ARRAY
};

enum class TextureWrapMode : GLint
{
    Repeat = GL_REPEAT,
    MirroredRepeat = GL_MIRRORED_REPEAT,
    ClampToEdge = GL_CLAMP_TO_EDGE,
    ClampToBorder = GL_CLAMP_TO_BORDER
};

enum class TextureFilterMode : GLint
{
    Nearest = GL_NEAREST,
    Linear = GL_LINEAR,
    NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
    NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
    LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
    LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
};

// Complete GL state description of a texture.
struct TextureDescription
{
    GLsizei width = 1;
    GLsizei height = 1;
    TextureType texture_type = TextureType::Texture2D;
    GLsizei layers = 1;                     // Only used by Texture2DArray / CubemapArray

    GLint internal_format = GL_RGB;
    GLenum format = GL_RGB;
    GLenum type = GL_UNSIGNED_BYTE;

    TextureWrapMode wrap_s = TextureWrapMode::Repeat;
    TextureWrapMode wrap_t = TextureWrapMode::Repeat;
    TextureWrapMode wrap_r = TextureWrapMode::Repeat;
    TextureFilterMode min_filter = TextureFilterMode::Linear;
    TextureFilterMode mag_filter = TextureFilterMode::Linear;

    bool gen_mipmap = false;

    friend bool operator==(const TextureDescription& lhs, const TextureDescription& rhs) = default;
};

class Texture
{
    friend class TextureFactory;
    friend class TextureLoader;
public:
    using Type = TextureType;

    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&&);
    Texture& operator=(Texture&&);

    void Bind(unsigned int unit = 0);
    void Unbind();

    TextureID GetID() const { return m_textureID; }
    Type GetType() const { return m_type; }

    const TextureDescription& GetDescription() const { return m_description; }
    bool HasStencil() const;

private:
    Texture(Type texture_type);

    Type m_type;
    TextureID m_textureID;
    int m_unit;
    TextureDescription m_description;
};

struct SurfaceTexture
{
    SurfaceTexture(const SurfaceTexture&) = delete;
    SurfaceTexture& operator=(const SurfaceTexture&) = delete;

    SurfaceTexture(SurfaceTexture&& other)
        : texture(std::move(other.texture)), type(std::move(other.type)) { }

    SurfaceTexture& operator=(SurfaceTexture&& other)
    {
        if (this != &other)
        {
            texture = std::move(other.texture);
            type = std::move(other.type);
        }
        return *this;
    }

    SurfaceTexture(Texture&& texture, const std::string& type)
        : texture(std::move(texture)), type(type) { }

    Texture texture;
    std::string type;
};

class TextureFactory
{
public:
    static std::optional<Texture> Create(const TextureDescription& description, const void* initial_data = nullptr);
};

// Options for loading a texture from image files.
struct TextureLoadOptions
{
    bool srgb = false;              // Promote RGB/RGBA to GL_SRGB / GL_SRGB_ALPHA
    bool high_precision = false;    // Use stbi_loadf, pixels are GL_FLOAT
    bool gen_mipmap = false;
    bool flip_vertically = true;
    TextureWrapMode wrap_s = TextureWrapMode::Repeat;
    TextureWrapMode wrap_t = TextureWrapMode::Repeat;
    TextureWrapMode wrap_r = TextureWrapMode::Repeat;
    TextureFilterMode min_filter = TextureFilterMode::Linear;
    TextureFilterMode mag_filter = TextureFilterMode::Linear;
};

// Loads textures from disk.
class TextureLoader
{
public:
    static std::optional<Texture> Load2D(const std::string& path, const TextureLoadOptions& options = {});
    static std::optional<Texture> LoadCubemap(const std::array<std::string, 6>& paths, const TextureLoadOptions& options = {});
};

} // namespace Aurora

namespace std
{
template<>
struct hash<Aurora::TextureDescription>
{
    std::size_t operator()(const Aurora::TextureDescription& description) const noexcept
    {
        std::size_t seed = 0;
        const auto combine = [&seed](auto value)
        {
            seed ^= std::hash<decltype(value)>{}(value) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
        };
        combine(description.width);
        combine(description.height);
        combine(description.texture_type);
        combine(description.layers);
        combine(description.internal_format);
        combine(description.format);
        combine(description.type);
        combine(description.wrap_s);
        combine(description.wrap_t);
        combine(description.wrap_r);
        combine(description.min_filter);
        combine(description.mag_filter);
        combine(description.gen_mipmap);
        return seed;
    }
};
} // namespace std
