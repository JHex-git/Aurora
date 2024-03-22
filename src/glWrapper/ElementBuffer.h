#pragma once
// std include
#include <vector>
// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
// Aurora include


namespace Aurora
{

class ElementBuffer
{
public:
    ElementBuffer();
    ~ElementBuffer();

    void Bind() const;
    void Unbind() const;

    template<typename T>
    bool LoadData(const std::vector<T> vertices, unsigned int num_vertices) const
    {
        if (vertices.empty())
        {
            return false;
        }

        Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_vertices * sizeof(T), vertices.data(), GL_STATIC_DRAW);

        Unbind();

        return true;
    }

private:
    unsigned int m_eboID;
};
} // namespace Aurora
