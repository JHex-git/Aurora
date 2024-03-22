// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
// Aurora include
#include "glWrapper/VertexBuffer.h"

namespace Aurora
{

VertexAttribPointer::VertexAttribPointer(unsigned int index, int size, unsigned int type, bool normalized, int stride, const void *pointer)
    : m_index(index), m_size(size), m_type(type), m_normalized(normalized), m_stride(stride), m_pointer(pointer)
{
}

VertexAttribPointer::~VertexAttribPointer()
{
}

void VertexAttribPointer::Enable() const
{
    glEnableVertexAttribArray(m_index);
    glVertexAttribPointer(m_index, m_size, m_type, m_normalized, m_stride, m_pointer);
}

void VertexAttribPointer::Disable() const
{
    glDisableVertexAttribArray(m_index);
}

VertexBuffer::VertexBuffer()
{
    glGenBuffers(1, &m_vboID);
    glGenVertexArrays(1, &m_vaoID);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_vboID);
    glDeleteVertexArrays(1, &m_vaoID);
}

void VertexBuffer::Bind() const
{
    glBindVertexArray(m_vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
}

void VertexBuffer::Unbind() const
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::SetAttribPointer(const VertexAttribPointer& attrib_pointer) const
{
    Bind();
    attrib_pointer.Enable();
    Unbind();
}

void VertexBuffer::SetAttribPointers(const std::vector<VertexAttribPointer>& attrib_pointers) const
{
    Bind();
    for (const auto& attrib_pointer : attrib_pointers)
    {
        attrib_pointer.Enable();
    }
    Unbind();
}


} // namespace Aurora