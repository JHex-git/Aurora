#pragma once
#include <string>

#include "glad/glad.h"

namespace Aurora
{

class Texture
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

    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&&);
    Texture& operator=(Texture&&);

    bool Load(const std::string& path, WrapType wrap_s = WrapType::Repeat, WrapType wrap_t = WrapType::Repeat, FilterType min_filter = FilterType::NearestMipmapLinear, FilterType mag_filter = FilterType::Linear, bool gen_mipmap = true, bool require_high_precision = false);
    void Bind(unsigned int unit = 0);
    void Unbind();

private:
    unsigned int m_textureID;
    int m_unit;
    int m_width;
    int m_height;
    int m_channels;
    bool m_isLoaded;
};
} // namespace Aurora
