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

struct ColorAttachmentDescriptor
{
    GLint internal_format = GL_RGB;
    GLenum format = GL_RGB;
    GLenum type = GL_UNSIGNED_BYTE;
    Texture::Type texture_type = Texture::Type::Texture2D;
};

struct DepthAttachmentDescriptor
{
    bool has_stencil = false;
    Texture::Type texture_type = Texture::Type::Texture2D;
    unsigned int layers = 1;
};

class FrameBufferObjectBuilder
{
public:
    FrameBufferObjectBuilder(GLsizei width, GLsizei height) : m_width(width), m_height(height) {}
    ~FrameBufferObjectBuilder() = default;

    FrameBufferObjectBuilder(const FrameBufferObjectBuilder&) = delete;
    FrameBufferObjectBuilder& operator=(const FrameBufferObjectBuilder&) = delete;

    FrameBufferObjectBuilder(FrameBufferObjectBuilder&&) = delete;
    FrameBufferObjectBuilder& operator=(FrameBufferObjectBuilder&&) = delete;

    FrameBufferObjectBuilder& AddColorAttachment(ColorAttachmentDescriptor&& descriptor);
    FrameBufferObjectBuilder& EnableDepthAttachment(DepthAttachmentDescriptor&& descriptor);

    std::optional<FrameBufferObject> Create();

private:
    GLsizei m_width;
    GLsizei m_height;
    std::vector<ColorAttachmentDescriptor> m_color_descriptors;
    std::optional<DepthAttachmentDescriptor> m_depth_descriptor;
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

    // face: 0 to 5
    void BindColorCubemapFace(size_t index, unsigned int face) const;
    // face: 0 to 5
    void BindColorCubemapFaceRead(size_t index, unsigned int face) const;
    // face: 0 to 5
    void BindColorCubemapFaceDraw(size_t index, unsigned int face) const;

    // face: 0 to 5
    void BindDepthCubemapFace(unsigned int face) const;
    // face: 0 to 5
    void BindDepthCubemapFaceRead(unsigned int face) const;
    // face: 0 to 5
    void BindDepthCubemapFaceDraw(unsigned int face) const;
    void BindDepthCubemapArray(unsigned int cubemap_index, unsigned int face) const;

    void BindColorTexture(size_t index, unsigned int unit);
    void BindDepthTexture(unsigned int unit);

    unsigned int GetColorAttachmentNumber() const { return m_color_attachments.size(); }
    TextureID GetColorAttachmentID(size_t index) const { return index < m_color_attachments.size() ? m_color_attachments[index].GetID() : 0; }
    TextureID GetDepthAttachmentID() const { return m_depth_stencil_attachment.has_value() ? m_depth_stencil_attachment.value().GetID() : 0; }

    std::vector<size_t> GetCubemapColorAttachmentIndices() const;

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
