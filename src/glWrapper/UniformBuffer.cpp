// std include

// thirdparty include

// Aurora include
#include "glWrapper/UniformBuffer.h"

namespace Aurora
{
UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding_point, GLenum usage, const void* data)
{
    glGenBuffers(1, &m_uboID);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
    glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_uboID);
}

UniformBuffer::~UniformBuffer()
{
    if (m_uboID != 0)
    {
        glDeleteBuffers(1, &m_uboID);
    }
}

void UniformBuffer::Bind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboID);
}

void UniformBuffer::Unbind() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset) const
{
    Bind();
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}
} // namespace Aurora
