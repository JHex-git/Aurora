// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
// Aurora include
#include "glWrapper/ElementBuffer.h"

namespace Aurora
{

ElementBuffer::ElementBuffer()
{
    glGenBuffers(1, &m_eboID);
}

ElementBuffer::~ElementBuffer()
{
    glDeleteBuffers(1, &m_eboID);
}

void ElementBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eboID);
}

void ElementBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
} // namespace Aurora