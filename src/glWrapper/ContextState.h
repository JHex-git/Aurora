#pragma once
// std include
#include <vector>
#include <array>
// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
// Aurora include


namespace Aurora
{

struct StencilFunc
{
    GLenum stencil_func = GL_ALWAYS;
    GLint stencil_ref = 0;
    GLuint stencil_read_mask = 0xFF;
};

struct StencilOp
{
    GLenum stencil_op_fail = GL_KEEP;
    GLenum stencil_op_zfail = GL_KEEP;
    GLenum stencil_op_zpass = GL_KEEP;
};

struct DepthStencilState
{
    uint32_t depth_test_enabled : 1 = true;
    uint32_t depth_write_enabled : 1 = true;
    uint32_t stencil_test_enabled : 1 = false;

    GLenum depth_func = GL_LESS;
    
    GLuint stencil_write_mask = 0xFF;
    StencilFunc stencil_func;
    StencilOp stencil_op;
};

struct BlendState
{
    bool blend_enabled = false;

    GLenum src_rgb   = GL_ONE;
    GLenum dst_rgb   = GL_ZERO;
    GLenum src_alpha = GL_ONE;
    GLenum dst_alpha = GL_ZERO;
    
    GLenum equation_rgb   = GL_FUNC_ADD;
    GLenum equation_alpha = GL_FUNC_ADD;

    static BlendState Additive();
};

struct ColorMask
{
    bool red   = true;
    bool green = true;
    bool blue  = true;
    bool alpha = true;

    static ColorMask WriteAll();
    static ColorMask WriteNone();
};

struct RasterState
{
    bool cull_enabled = true;
    GLenum cull_face  = GL_BACK;
    GLenum front_face = GL_CCW;
};

struct RenderState
{
    DepthStencilState depth_stencil_state;
    BlendState blend_state;
    ColorMask color_mask;
    RasterState raster_state;
};

class ContextState
{
public:
    ContextState() = default;
    ~ContextState() = default;

    void ApplyRenderState(const RenderState&);
    void Invalidate();
private:
    void SetDepthStencilState(const DepthStencilState&, bool force = false);
    void SetBlendState(const BlendState&, bool force = false);
    void SetColorMask(const ColorMask&, bool force = false);
    void SetRasterState(const RasterState&, bool force = false);

    bool m_dirty = true;
    RenderState m_render_state;
};
} // namespace Aurora
