// std include

// thirdparty include
// Aurora include
#include "glWrapper/Utils.h"
#include "glWrapper/FrameBufferObject.h"

namespace Aurora
{

void Blit(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLbitfield mask, GLenum filter)
{
    src->BindRead();
    dst->BindDraw();
    const auto&& [src_width, src_height] = src->GetSize();
    const auto&& [dst_width, dst_height] = dst->GetSize();
    glBlitFramebuffer(0, 0, src_width, src_height, 0, 0, dst_width, dst_height, mask, filter);
    src->UnbindRead();
    dst->UnbindDraw();
}

void BlitColor(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLenum filter)
{
    Blit(src, dst, GL_COLOR_BUFFER_BIT, filter);
}

void BlitDepth(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLenum filter)
{
    Blit(src, dst, GL_DEPTH_BUFFER_BIT, filter);
}

void BlitStencil(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLenum filter)
{
    Blit(src, dst, GL_STENCIL_BUFFER_BIT, filter);
}
} // namespace Aurora