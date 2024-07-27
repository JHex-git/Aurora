// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "glWrapper/FrameBufferObject.h"
#include "glWrapper/Texture.h"


namespace Aurora
{

FrameBufferObjectBuilder& FrameBufferObjectBuilder::AddColorAttachment(GLint internal_format)
{
    m_color_internal_formats.push_back(internal_format);
    return *this;
}

FrameBufferObjectBuilder& FrameBufferObjectBuilder::EnableDepthAttachment()
{
    m_is_depth_enabled = true;
    return *this;
}

FrameBufferObjectBuilder& FrameBufferObjectBuilder::EnableStencilAttachment()
{
    m_is_stencil_enabled = true;
    return *this;
}

std::optional<FrameBufferObject> FrameBufferObjectBuilder::Create()
{
    FrameBufferObject fbo;

    fbo.Bind();

    // Create color attachments
    for (size_t i = 0; i < m_color_internal_formats.size(); ++i)
    {
        auto color_attachment = TextureBuilder().WithInternalFormat(m_color_internal_formats[i]).MakeTexture2D(m_width, m_height);
        if (color_attachment)
        {
            color_attachment->Bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_attachment->GetID(), 0);
            fbo.m_color_attachments.push_back(std::move(*color_attachment));
        }
        else
        {
            spdlog::error("Failed to create color attachment {}", i);
            return std::nullopt;
        }
    }

    // Create depth attachment
    if (m_is_depth_enabled)
    {
        auto depth_attachment = TextureBuilder().WithInternalFormat(GL_DEPTH_COMPONENT).MakeTexture2D(m_width, m_height);
        if (depth_attachment)
        {
            depth_attachment->Bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment->GetID(), 0);
            fbo.m_depth_attachment = std::move(*depth_attachment);
        }
        else
        {
            spdlog::error("Failed to create depth attachment");
            return std::nullopt;   
        }
    }

    // Create stencil attachment
    if (m_is_stencil_enabled)
    {
        auto stencil_attachment = TextureBuilder().WithInternalFormat(GL_STENCIL_INDEX).MakeTexture2D(m_width, m_height);
        if (stencil_attachment)
        {
            stencil_attachment->Bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencil_attachment->GetID(), 0);
            fbo.m_stencil_attachment = std::move(*stencil_attachment);
        }
        else
        {
            spdlog::error("Failed to create stencil attachment");
            return std::nullopt;
        }
    }

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        spdlog::error("Framebuffer is not complete, status: {:x}", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return std::nullopt;
    }

    fbo.Unbind();

    return std::make_optional<FrameBufferObject>(std::move(fbo));
}

FrameBufferObject::FrameBufferObject()
{
    glGenFramebuffers(1, &m_fboID);
}

FrameBufferObject::~FrameBufferObject()
{
    glDeleteFramebuffers(1, &m_fboID);
}

FrameBufferObject::FrameBufferObject(FrameBufferObject&& other) noexcept
{
    m_fboID = other.m_fboID;
    m_color_attachments = std::move(other.m_color_attachments);
    m_depth_attachment = std::move(other.m_depth_attachment);
    m_stencil_attachment = std::move(other.m_stencil_attachment);
    other.m_fboID = 0;
}

FrameBufferObject& FrameBufferObject::operator=(FrameBufferObject&& other) noexcept
{
    if (this != &other)
    {
        glDeleteFramebuffers(1, &m_fboID);
        m_fboID = other.m_fboID;
        m_color_attachments = std::move(other.m_color_attachments);
        m_depth_attachment = std::move(other.m_depth_attachment);
        m_stencil_attachment = std::move(other.m_stencil_attachment);
        other.m_fboID = 0;
    }
    return *this;
}

void FrameBufferObject::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
}

void FrameBufferObject::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
}// namespace Aurora