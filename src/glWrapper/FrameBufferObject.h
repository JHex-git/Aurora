#pragma once

namespace Aurora
{

class FrameBufferObject
{
public:
    FrameBufferObject();
    ~FrameBufferObject();

    void Bind() const;
    void Unbind() const;

private:
    unsigned int m_fboID;
};
} // namespace Aurora
