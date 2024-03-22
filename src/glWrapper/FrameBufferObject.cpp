// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
// Aurora include
#include "glWrapper/FrameBufferObject.h"


namespace Aurora
{

FrameBufferObject::FrameBufferObject()
{
    glGenFramebuffers(1, &m_fboID);
}

FrameBufferObject::~FrameBufferObject()
{
    glDeleteFramebuffers(1, &m_fboID);
}
}// namespace Aurora