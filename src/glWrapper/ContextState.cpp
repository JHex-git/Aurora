// std include

// thirdparty include

// Aurora include
#include "glWrapper/ContextState.h"

namespace Aurora
{

inline bool operator!=(const StencilFunc& a, const StencilFunc& b)
{
    return a.stencil_func != b.stencil_func ||
           a.stencil_ref != b.stencil_ref ||
           a.stencil_read_mask != b.stencil_read_mask;
}

inline bool operator!=(const StencilOp& a, const StencilOp& b)
{
    return a.stencil_op_fail != b.stencil_op_fail ||
           a.stencil_op_zfail != b.stencil_op_zfail ||
           a.stencil_op_zpass != b.stencil_op_zpass;
}


BlendState BlendState::Additive()
{
    BlendState state;
    state.blend_enabled = true;
    state.src_rgb = GL_ONE;
    state.dst_rgb = GL_ONE;
    state.src_alpha = GL_ZERO;
    state.dst_alpha = GL_ONE;
    state.equation_rgb = GL_FUNC_ADD;
    state.equation_alpha = GL_FUNC_ADD;
    return state;
}

ColorMask ColorMask::WriteAll()
{
    ColorMask mask = {
        .red = true,
        .green = true,
        .blue = true,
        .alpha = true
    };
    return mask;
}

ColorMask ColorMask::WriteNone()
{
    ColorMask mask = {
        .red = false,
        .green = false,
        .blue = false,
        .alpha = false
    };
    return mask;
}

void ContextState::ApplyRenderState(const RenderState& new_state)
{
    bool should_force = m_dirty;
    SetDepthStencilState(new_state.depth_stencil_state, should_force);
    SetBlendState(new_state.blend_state, should_force);
    SetColorMask(new_state.color_mask, should_force);
    SetRasterState(new_state.raster_state, should_force);
    m_dirty = false;
}

void ContextState::Invalidate()
{
    m_dirty = true;
}

void ContextState::SetDepthStencilState(const DepthStencilState& new_state, bool force)
{
    // -- Depth --
    if (force || m_render_state.depth_stencil_state.depth_test_enabled != new_state.depth_test_enabled)
    {
        if (new_state.depth_test_enabled) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
        m_render_state.depth_stencil_state.depth_test_enabled = new_state.depth_test_enabled;
    }

    if (force || m_render_state.depth_stencil_state.depth_write_enabled != new_state.depth_write_enabled)
    {
        glDepthMask(new_state.depth_write_enabled ? GL_TRUE : GL_FALSE);
        m_render_state.depth_stencil_state.depth_write_enabled = new_state.depth_write_enabled;
    }

    if (new_state.depth_test_enabled)
    {
        if (force || m_render_state.depth_stencil_state.depth_func != new_state.depth_func)
        {
            glDepthFunc(new_state.depth_func);
            m_render_state.depth_stencil_state.depth_func = new_state.depth_func;
        }
    }

    // -- Stencil --
    if (force || m_render_state.depth_stencil_state.stencil_test_enabled != new_state.stencil_test_enabled)
    {
        if (new_state.stencil_test_enabled) glEnable(GL_STENCIL_TEST);
        else glDisable(GL_STENCIL_TEST);
        m_render_state.depth_stencil_state.stencil_test_enabled = new_state.stencil_test_enabled;
    }

    if (new_state.stencil_test_enabled)
    {
        if (force || m_render_state.depth_stencil_state.stencil_write_mask != new_state.stencil_write_mask)
        {
            glStencilMask(new_state.stencil_write_mask);
            m_render_state.depth_stencil_state.stencil_write_mask = new_state.stencil_write_mask;
        }

        if (force || m_render_state.depth_stencil_state.stencil_func != new_state.stencil_func)
        {
            glStencilFunc(
                new_state.stencil_func.stencil_func, 
                new_state.stencil_func.stencil_ref, 
                new_state.stencil_func.stencil_read_mask
            );
            
            m_render_state.depth_stencil_state.stencil_func = new_state.stencil_func;
        }

        if (force || m_render_state.depth_stencil_state.stencil_op != new_state.stencil_op)
        {
            glStencilOp(
                new_state.stencil_op.stencil_op_fail,
                new_state.stencil_op.stencil_op_zfail,
                new_state.stencil_op.stencil_op_zpass
            );

            m_render_state.depth_stencil_state.stencil_op = new_state.stencil_op;
        }
    }
}

void ContextState::SetBlendState(const BlendState& new_state, bool force)
{
    if (force || m_render_state.blend_state.blend_enabled != new_state.blend_enabled)
    {
        if (new_state.blend_enabled) glEnable(GL_BLEND);
        else glDisable(GL_BLEND);
        
        m_render_state.blend_state.blend_enabled = new_state.blend_enabled;
    }

    if (new_state.blend_enabled)
    {
        if (force || 
            m_render_state.blend_state.src_rgb   != new_state.src_rgb   ||
            m_render_state.blend_state.dst_rgb   != new_state.dst_rgb   ||
            m_render_state.blend_state.src_alpha != new_state.src_alpha ||
            m_render_state.blend_state.dst_alpha != new_state.dst_alpha)
        {
            glBlendFuncSeparate(
                new_state.src_rgb, 
                new_state.dst_rgb, 
                new_state.src_alpha, 
                new_state.dst_alpha
            );

            m_render_state.blend_state.src_rgb   = new_state.src_rgb;
            m_render_state.blend_state.dst_rgb   = new_state.dst_rgb;
            m_render_state.blend_state.src_alpha = new_state.src_alpha;
            m_render_state.blend_state.dst_alpha = new_state.dst_alpha;
        }

        if (force || 
            m_render_state.blend_state.equation_rgb   != new_state.equation_rgb ||
            m_render_state.blend_state.equation_alpha != new_state.equation_alpha)
        {
            glBlendEquationSeparate(new_state.equation_rgb, new_state.equation_alpha);
            
            m_render_state.blend_state.equation_rgb   = new_state.equation_rgb;
            m_render_state.blend_state.equation_alpha = new_state.equation_alpha;
        }
    }
}

void ContextState::SetColorMask(const ColorMask& new_mask, bool force)
{
    if (force || 
        m_render_state.color_mask.red   != new_mask.red   ||
        m_render_state.color_mask.green != new_mask.green ||
        m_render_state.color_mask.blue  != new_mask.blue  ||
        m_render_state.color_mask.alpha != new_mask.alpha)
    {
        glColorMask(
            new_mask.red ? GL_TRUE : GL_FALSE,
            new_mask.green ? GL_TRUE : GL_FALSE,
            new_mask.blue ? GL_TRUE : GL_FALSE,
            new_mask.alpha ? GL_TRUE : GL_FALSE
        );

        m_render_state.color_mask = new_mask;
    }
}

void ContextState::SetRasterState(const RasterState& new_state, bool force)
{
    if (force || m_render_state.raster_state.cull_enabled != new_state.cull_enabled)
    {
        if (new_state.cull_enabled) glEnable(GL_CULL_FACE);
        else glDisable(GL_CULL_FACE);
        m_render_state.raster_state.cull_enabled = new_state.cull_enabled;
    }

    if (new_state.cull_enabled)
    {
        if (force || m_render_state.raster_state.cull_face != new_state.cull_face)
        {
            glCullFace(new_state.cull_face);
            m_render_state.raster_state.cull_face = new_state.cull_face;
        }

        if (force || m_render_state.raster_state.front_face != new_state.front_face)
        {
            glFrontFace(new_state.front_face);
            m_render_state.raster_state.front_face = new_state.front_face;
        }
    }
}

} // namespace Aurora