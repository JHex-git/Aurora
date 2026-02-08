#pragma once
#include <vector>
#include "thirdparty/opengl/glad/include/glad/glad.h"

namespace Aurora
{

class VertexAttribPointer
{
public:
    VertexAttribPointer(unsigned int index, int size, unsigned int type, bool normalized, int stride, const void *pointer);
    ~VertexAttribPointer();

    void Enable() const;
    void Disable() const;

private:
    unsigned int m_index;
    int m_size;
    unsigned int m_type;
    bool m_normalized;
    int m_stride;
    const void *m_pointer;
};

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    void SetAttribPointer(const VertexAttribPointer& attrib_pointer) const;
    void SetAttribPointers(const std::vector<VertexAttribPointer>& attrib_pointer) const;

private:
    unsigned int m_vaoID;
};

} // namespace Aurora
