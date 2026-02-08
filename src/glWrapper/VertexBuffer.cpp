// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
// Aurora include
#include "glWrapper/VertexBuffer.h"

namespace Aurora
{

VertexBuffer::VertexBuffer()
{
    glGenBuffers(1, &m_vboID);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_vboID);
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
}

void VertexBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace Aurora