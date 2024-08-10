#pragma once
// std include
#include <memory>
// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
// Aurora include


namespace Aurora
{
class FrameBufferObject;

void Blit(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GLenum filter = GL_NEAREST);

void BlitColor(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLenum filter = GL_NEAREST);

void BlitDepth(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLenum filter = GL_NEAREST);

void BlitStencil(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLenum filter = GL_NEAREST);
} // namespace Aurora