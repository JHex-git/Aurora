#pragma once
// std include
#include <optional>
#include <vector>
#include <array>
// thirdparty include

// Aurora include
#include "glWrapper/Texture.h"

namespace Aurora
{
class FrameBufferObject;

class FrameBufferObjectBuilder
{
public:
    FrameBufferObjectBuilder(GLsizei width, GLsizei height) : m_width(width), m_height(height) {}
    ~FrameBufferObjectBuilder() = default;

    FrameBufferObjectBuilder(const FrameBufferObjectBuilder&) = delete;
    FrameBufferObjectBuilder& operator=(const FrameBufferObjectBuilder&) = delete;

    FrameBufferObjectBuilder(FrameBufferObjectBuilder&&) = delete;
    FrameBufferObjectBuilder& operator=(FrameBufferObjectBuilder&&) = delete;

    FrameBufferObjectBuilder& AddColorAttachment(GLint internal_format = GL_RGB);
    FrameBufferObjectBuilder& EnableDepthAttachmentOnly();
    FrameBufferObjectBuilder& EnableDepthStencilAttachment();

    std::optional<FrameBufferObject> Create();

private:
    bool m_is_depth_enabled = false;
    bool m_is_stencil_enabled = false;

    GLsizei m_width;
    GLsizei m_height;
    std::vector<GLint> m_color_internal_formats;
};

class FrameBufferObject
{
    friend class FrameBufferObjectBuilder;
public:
    ~FrameBufferObject();

    FrameBufferObject(const FrameBufferObject&) = delete;
    FrameBufferObject& operator=(const FrameBufferObject&) = delete;

    FrameBufferObject(FrameBufferObject&&) noexcept;
    FrameBufferObject& operator=(FrameBufferObject&&) noexcept;

    void Bind() const;
    void Unbind() const;

    void BindRead() const;
    void UnbindRead() const;

    void BindDraw() const;
    void UnbindDraw() const;

    unsigned int GetColorAttachmentNumber() const { return m_color_attachments.size(); }
    int GetColorAttachmentID(size_t index) const { return index < m_color_attachments.size() ? m_color_attachments[index].GetID() : -1; }
    int GetDepthStencilAttachmentID() const { return m_depth_stencil_attachment.has_value() ? m_depth_stencil_attachment.value().GetID() : -1; }

    unsigned int GetID() const { return m_fboID; }
    const std::array<unsigned int, 2> GetSize() const { return { m_width, m_height }; }

private:
    FrameBufferObject(unsigned int width, unsigned int height);

    unsigned int m_fboID;
    unsigned int m_width;
    unsigned int m_height;
    std::vector<Texture>    m_color_attachments;
    std::optional<Texture>  m_depth_stencil_attachment;
};
} // namespace Aurora
