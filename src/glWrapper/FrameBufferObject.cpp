// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "glWrapper/FrameBufferObject.h"
#include "glWrapper/Texture.h"


namespace Aurora
{

std::optional<FrameBufferObject> FrameBufferObject::Create(GLsizei width,
                                                          GLsizei height,
                                                          const std::vector<Texture*>& color_attachments,
                                                          Texture* depth_attachment)
{
    if (width <= 0 || height <= 0)
    {
        spdlog::error("FrameBufferObject::Create: invalid size {}x{}", width, height);
        return std::nullopt;
    }
    if (color_attachments.empty() && depth_attachment == nullptr)
    {
        spdlog::error("FrameBufferObject::Create: no attachments provided");
        return std::nullopt;
    }
    for (size_t i = 0; i < color_attachments.size(); ++i)
    {
        if (color_attachments[i] == nullptr)
        {
            spdlog::error("FrameBufferObject::Create: color attachment {} is null", i);
            return std::nullopt;
        }
    }

    FrameBufferObject fbo(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
    fbo.m_color_attachments.assign(color_attachments.begin(), color_attachments.end());
    fbo.m_depth_attachment = depth_attachment;
    fbo.m_depth_attachment_point = (depth_attachment != nullptr && depth_attachment->HasStencil())
        ? GL_DEPTH_STENCIL_ATTACHMENT
        : GL_DEPTH_ATTACHMENT;

    fbo.Bind();
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        spdlog::error("FrameBufferObject::Create: framebuffer is not complete, status: {:x}", status);
        return std::nullopt;
    }
    fbo.Unbind();

    return fbo;
}

FrameBufferObject::FrameBufferObject(unsigned int width, unsigned int height)
    : m_width(width), m_height(height)
{
    glGenFramebuffers(1, &m_fboID);
}

void FrameBufferObject::AttachAll(GLenum target) const
{
    const bool has_draw_side = target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER;
    const bool has_read_side = target == GL_FRAMEBUFFER || target == GL_READ_FRAMEBUFFER;

    if (has_draw_side)
    {
        if (m_color_attachments.empty())
        {
            glDrawBuffer(GL_NONE);
        }
        else
        {
            std::vector<GLenum> draw_buffers(m_color_attachments.size());
            for (size_t i = 0; i < draw_buffers.size(); ++i)
            {
                draw_buffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
            }
            glDrawBuffers(static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data());
        }
    }
    if (has_read_side && m_color_attachments.empty())
    {
        glReadBuffer(GL_NONE);
    }

    for (size_t i = 0; i < m_color_attachments.size(); ++i)
    {
        const GLenum attachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
        const Texture* texture = m_color_attachments[i];
        if (texture->GetType() == Texture::Type::Texture2D)
            glFramebufferTexture2D(target, attachment, GL_TEXTURE_2D, texture->GetID(), 0);
        else
            glFramebufferTexture(target, attachment, texture->GetID(), 0);
    }

    if (m_depth_attachment != nullptr)
    {
        if (m_depth_attachment->GetType() == Texture::Type::Texture2D)
            glFramebufferTexture2D(target, m_depth_attachment_point, GL_TEXTURE_2D, m_depth_attachment->GetID(), 0);
        else
            glFramebufferTexture(target, m_depth_attachment_point, m_depth_attachment->GetID(), 0);
    }
}

FrameBufferObject::~FrameBufferObject()
{
    glDeleteFramebuffers(1, &m_fboID);
}

FrameBufferObject::FrameBufferObject(FrameBufferObject&& other) noexcept
{
    m_fboID = other.m_fboID;
    m_color_attachments = std::move(other.m_color_attachments);
    m_depth_attachment = other.m_depth_attachment;
    m_depth_attachment_point = other.m_depth_attachment_point;
    m_width = other.m_width;
    m_height = other.m_height;
    other.m_fboID = 0;
    other.m_depth_attachment = nullptr;
}

FrameBufferObject& FrameBufferObject::operator=(FrameBufferObject&& other) noexcept
{
    if (this != &other)
    {
        glDeleteFramebuffers(1, &m_fboID);
        m_fboID = other.m_fboID;
        m_color_attachments = std::move(other.m_color_attachments);
        m_depth_attachment = other.m_depth_attachment;
        m_depth_attachment_point = other.m_depth_attachment_point;
        m_width = other.m_width;
        m_height = other.m_height;
        other.m_fboID = 0;
        other.m_depth_attachment = nullptr;
    }
    return *this;
}

void FrameBufferObject::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
    AttachAll(GL_FRAMEBUFFER);
    glViewport(0, 0, m_width, m_height);
}

void FrameBufferObject::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferObject::BindRead() const
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
    AttachAll(GL_READ_FRAMEBUFFER);
}

void FrameBufferObject::UnbindRead() const
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void FrameBufferObject::BindDraw() const
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
    AttachAll(GL_DRAW_FRAMEBUFFER);
}

void FrameBufferObject::UnbindDraw() const
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBufferObject::BindColorCubemapFace(size_t index, unsigned int face) const
{
    const Texture* texture = GetColorAttachment(index);
    if (texture != nullptr && texture->GetType() == Texture::Type::Cubemap)
    {
        Bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index), GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture->GetID(), 0);
    }
}

void FrameBufferObject::BindColorCubemapFaceRead(size_t index, unsigned int face) const
{
    const Texture* texture = GetColorAttachment(index);
    if (texture != nullptr && texture->GetType() == Texture::Type::Cubemap)
    {
        BindRead();
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index), GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture->GetID(), 0);
    }
}

void FrameBufferObject::BindColorCubemapFaceDraw(size_t index, unsigned int face) const
{
    const Texture* texture = GetColorAttachment(index);
    if (texture != nullptr && texture->GetType() == Texture::Type::Cubemap)
    {
        BindDraw();
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index), GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture->GetID(), 0);
    }
}

void FrameBufferObject::BindDepthCubemapFace(unsigned int face) const
{
    const Texture* depth = GetDepthAttachment();
    if (depth != nullptr && depth->GetType() == Texture::Type::Cubemap)
    {
        Bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, m_depth_attachment_point, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, depth->GetID(), 0);
    }
}

void FrameBufferObject::BindDepthCubemapFaceRead(unsigned int face) const
{
    const Texture* depth = GetDepthAttachment();
    if (depth != nullptr && depth->GetType() == Texture::Type::Cubemap)
    {
        BindRead();
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, m_depth_attachment_point, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, depth->GetID(), 0);
    }
}

void FrameBufferObject::BindDepthCubemapFaceDraw(unsigned int face) const
{
    const Texture* depth = GetDepthAttachment();
    if (depth != nullptr && depth->GetType() == Texture::Type::Cubemap)
    {
        BindDraw();
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, m_depth_attachment_point, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, depth->GetID(), 0);
    }
}

void FrameBufferObject::BindDepthCubemapArray(unsigned int cubemap_index, unsigned int face) const
{
    const Texture* depth = GetDepthAttachment();
    if (depth != nullptr && depth->GetType() == Texture::Type::CubemapArray)
    {
        Bind();
        glFramebufferTextureLayer(GL_FRAMEBUFFER, m_depth_attachment_point, depth->GetID(), 0, cubemap_index * 6 + face);
    }
}

void FrameBufferObject::BindDepthTextureLayer(unsigned int layer) const
{
    const Texture* depth = GetDepthAttachment();
    if (depth != nullptr && depth->GetType() == Texture::Type::Texture2DArray)
    {
        Bind();
        glFramebufferTextureLayer(GL_FRAMEBUFFER, m_depth_attachment_point, depth->GetID(), 0, layer);
    }
}


void FrameBufferObject::BindColorTexture(size_t index, unsigned int unit)
{
    if (Texture* texture = GetColorAttachment(index))
    {
        texture->Bind(unit);
    }
}

void FrameBufferObject::BindDepthTexture(unsigned int unit)
{
    if (Texture* texture = GetDepthAttachment())
    {
        texture->Bind(unit);
    }
}

std::vector<size_t> FrameBufferObject::GetCubemapColorAttachmentIndices() const
{
    std::vector<size_t> indices;
    const size_t color_count = GetColorAttachmentNumber();
    for (size_t i = 0; i < color_count; ++i)
    {
        const Texture* texture = GetColorAttachment(i);
        if (texture != nullptr && texture->GetType() == Texture::Type::Cubemap)
            indices.push_back(i);
    }
    return indices;
}
}// namespace Aurora
