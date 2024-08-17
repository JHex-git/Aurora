#pragma once
// std include
#include <vector>
#include <string>
#include <optional>
// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
// Aurora include

namespace Aurora
{

class Texture;
using TextureID = GLuint;

class TextureBuilder
{
public:
    enum class WrapType : GLint
    {
        Repeat = GL_REPEAT,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        ClampToEdge = GL_CLAMP_TO_EDGE,
        ClampToBorder = GL_CLAMP_TO_BORDER
    };

    enum class FilterType : GLint
    {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
        NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
        NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
        LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
    };

    enum class CubeFace : GLenum
    {
        PosX = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        NegX = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        PosY = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        NegY = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        PosZ = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        NegZ = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    TextureBuilder() = default;
    ~TextureBuilder() = default;

    TextureBuilder(const TextureBuilder&) = delete;
    TextureBuilder& operator=(const TextureBuilder&) = delete;

    TextureBuilder(TextureBuilder&&) = delete;
    TextureBuilder& operator=(TextureBuilder&&) = delete;

    TextureBuilder& WithInternalFormat(GLint internal_format)
    {
        m_internal_format = internal_format;
        return *this;
    }

    TextureBuilder& WithWrapS(WrapType wrap_s)
    {
        m_wrap_s = wrap_s;
        return *this;
    }

    TextureBuilder& WithWrapT(WrapType wrap_t)
    {
        m_wrap_t = wrap_t;
        return *this;
    }

    TextureBuilder& WithWrapR(WrapType wrap_r)
    {
        m_wrap_r = wrap_r;
        return *this;
    }

    TextureBuilder& WithMinFilter(FilterType min_filter)
    {
        m_min_filter = min_filter;
        return *this;
    }

    TextureBuilder& WithMagFilter(FilterType mag_filter)
    {
        m_mag_filter = mag_filter;
        return *this;
    }

    TextureBuilder& WithGenMipmap(bool gen_mipmap)
    {
        m_gen_mipmap = gen_mipmap;
        return *this;
    }

    TextureBuilder& WithRequireHighPrecision(bool require_high_precision)
    {
        m_require_high_precision = require_high_precision;
        return *this;
    }

    std::optional<Texture> MakeTexture2D(GLsizei width, GLsizei height, GLint format, GLenum type);
    std::optional<Texture> MakeTexture2D(const std::string& path);

    std::optional<Texture> MakeTextureCubeMap(GLsizei width, GLsizei height, GLint format, GLenum type);
    std::optional<Texture> MakeTextureCubeMap(const std::array<std::string, 6>& paths);

private:
    WrapType m_wrap_s = WrapType::Repeat;
    WrapType m_wrap_t = WrapType::Repeat;
    WrapType m_wrap_r = WrapType::Repeat;
    FilterType m_min_filter = FilterType::Linear;
    FilterType m_mag_filter = FilterType::Linear;
    bool m_gen_mipmap = false;
    bool m_require_high_precision = false;
    GLint m_internal_format = GL_RGB;
};

class Texture
{
    friend class TextureBuilder;
    friend class TextureManager;
public:
    enum class Type : GLenum
    {
        Texture2D = GL_TEXTURE_2D,
        Cubemap = GL_TEXTURE_CUBE_MAP
    };

    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&&);
    Texture& operator=(Texture&&);

    void Bind(unsigned int unit = 0);
    void Unbind();

    unsigned int GetID() const { return m_textureID; }
    Type GetType() const { return m_type; }

private:
    Texture(Type texture_type);
    
    Type m_type;
    TextureID m_textureID;
    int m_unit;
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

} // namespace Aurora
