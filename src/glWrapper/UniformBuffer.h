#pragma once
#include <vector>
#include "thirdparty/opengl/glad/include/glad/glad.h"

namespace Aurora
{

class UniformBuffer
{
public:
    UniformBuffer(uint32_t size, uint32_t binding_point, GLenum usage, const void* data = nullptr);
    ~UniformBuffer();

    UniformBuffer(const UniformBuffer&) = delete;
    UniformBuffer& operator=(const UniformBuffer&) = delete;

    void Bind() const;
    void Unbind() const;

    void SetData(const void* data, uint32_t size, uint32_t offset = 0) const;
private:
    unsigned int m_uboID;
};

} // namespace Aurora
