// std include

// thirdparty include
#include "thirdparty/opengl/glad/include/glad/glad.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "glWrapper/FrameBufferObject.h"
#include "glWrapper/Texture.h"


namespace Aurora
{

FrameBufferObjectBuilder& FrameBufferObjectBuilder::AddColorAttachment(ColorAttachmentDescriptor&& descriptor)
{
    m_color_descriptors.push_back(std::move(descriptor));
    return *this;
}

FrameBufferObjectBuilder& FrameBufferObjectBuilder::EnableDepthAttachment(DepthAttachmentDescriptor&& descriptor)
{
    if (descriptor.has_stencil && descriptor.texture_type == Texture::Type::Cubemap)
    {
        spdlog::error("Cubemap texture does not support stencil attachment");
        return *this;
    }
    m_depth_descriptor = std::move(descriptor);
    return *this;
}

std::optional<FrameBufferObject> FrameBufferObjectBuilder::Create()
{
    FrameBufferObject fbo(m_width, m_height);

    fbo.Bind();

    if (m_color_descriptors.empty())
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    else
    {
        // Create color attachments
        for (size_t i = 0; i < m_color_descriptors.size(); ++i)
        {
            auto color_attachment = m_color_descriptors[i].texture_type == 
                Texture::Type::Texture2D ?  
                    TextureBuilder().WithInternalFormat(m_color_descriptors[i].internal_format).MakeTexture2D(m_width, m_height, m_color_descriptors[i].format, m_color_descriptors[i].type) :
                    TextureBuilder().WithInternalFormat(m_color_descriptors[i].internal_format)
                                    .WithWrapS(TextureBuilder::WrapType::ClampToEdge)
                                    .WithWrapT(TextureBuilder::WrapType::ClampToEdge)
                                    .WithWrapR(TextureBuilder::WrapType::ClampToEdge)
                                    .MakeTextureCubeMap(m_width, m_height, m_color_descriptors[i].format, m_color_descriptors[i].type);
            if (color_attachment)
            {
                color_attachment->Bind();
                if (m_color_descriptors[i].texture_type == Texture::Type::Texture2D)
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_attachment->GetID(), 0);
                else
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color_attachment->GetID(), 0);
                fbo.m_color_attachments.push_back(std::move(*color_attachment));
            }
            else
            {
                spdlog::error("Failed to create color attachment {}", i);
                return std::nullopt;
            }
        }
    }

    if (m_depth_descriptor.has_value())
    {
        // has depth and stencil
        if (m_depth_descriptor->has_stencil)
        {
            auto depth_stencil_attachment = TextureBuilder().WithInternalFormat(GL_DEPTH_STENCIL).MakeTexture2D(m_width, m_height, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
            if (depth_stencil_attachment)
            {
                depth_stencil_attachment->Bind();
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_stencil_attachment->GetID(), 0);
                fbo.m_depth_stencil_attachment = std::move(*depth_stencil_attachment);
            }
            else
            {
                spdlog::error("Failed to create DepthStencil attachment");
                return std::nullopt;
            }
        }
        else // has only depth
        {
            if (m_depth_descriptor->texture_type == Texture::Type::Cubemap || m_depth_descriptor->texture_type == Texture::Type::CubemapArray)
            {
                TextureBuilder& texture_builder = TextureBuilder().WithInternalFormat(GL_DEPTH_COMPONENT)
                                                        .WithWrapS(TextureBuilder::WrapType::ClampToEdge)
                                                        .WithWrapT(TextureBuilder::WrapType::ClampToEdge)
                                                        .WithWrapR(TextureBuilder::WrapType::ClampToEdge);
                std::optional<Texture> depth_attachment;
                if (m_depth_descriptor->texture_type == Texture::Type::Cubemap)
                {
                    depth_attachment = texture_builder.MakeTextureCubeMap(m_width, m_height, GL_DEPTH_COMPONENT, GL_FLOAT);
                }
                else
                {
                    depth_attachment = texture_builder.MakeTextureCubeMapArray(m_width, m_height, GL_DEPTH_COMPONENT, GL_FLOAT, m_depth_descriptor->layers);
                }

                if (depth_attachment)
                {
                    depth_attachment->Bind();
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_attachment->GetID(), 0);
                    fbo.m_depth_stencil_attachment = std::move(*depth_attachment);
                }
            }
            else if (m_depth_descriptor->texture_type == Texture::Type::Texture2D)
            {
                auto depth_attachment = TextureBuilder().WithInternalFormat(GL_DEPTH_COMPONENT).MakeTexture2D(m_width, m_height, GL_DEPTH_COMPONENT, GL_FLOAT);
                if (depth_attachment)
                {
                    depth_attachment->Bind();
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_attachment->GetID(), 0);
                    fbo.m_depth_stencil_attachment = std::move(*depth_attachment);
                }
            }
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

FrameBufferObject::FrameBufferObject(unsigned int width, unsigned int height)
    : m_width(width), m_height(height)
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
    m_depth_stencil_attachment = std::move(other.m_depth_stencil_attachment);
    m_width = other.m_width;
    m_height = other.m_height;
    other.m_fboID = 0;
}

FrameBufferObject& FrameBufferObject::operator=(FrameBufferObject&& other) noexcept
{
    if (this != &other)
    {
        glDeleteFramebuffers(1, &m_fboID);
        m_fboID = other.m_fboID;
        m_color_attachments = std::move(other.m_color_attachments);
        m_depth_stencil_attachment = std::move(other.m_depth_stencil_attachment);
        m_width = other.m_width;
        m_height = other.m_height;
        other.m_fboID = 0;
    }
    return *this;
}

void FrameBufferObject::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
    glViewport(0, 0, m_width, m_height);
}

void FrameBufferObject::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferObject::BindRead() const
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
}

void FrameBufferObject::UnbindRead() const
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void FrameBufferObject::BindDraw() const
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID);
}

void FrameBufferObject::UnbindDraw() const
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBufferObject::BindColorCubemapFace(size_t index, unsigned int face) const
{
    if (index < m_color_attachments.size() && m_color_attachments[index].GetType() == Texture::Type::Cubemap)
    {
        Bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_color_attachments[index].GetID(), 0);
    }
}

void FrameBufferObject::BindColorCubemapFaceRead(size_t index, unsigned int face) const
{
    if (index < m_color_attachments.size() && m_color_attachments[index].GetType() == Texture::Type::Cubemap)
    {
        BindRead();
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_color_attachments[index].GetID(), 0);
    }
}

void FrameBufferObject::BindColorCubemapFaceDraw(size_t index, unsigned int face) const
{
    if (index < m_color_attachments.size() && m_color_attachments[index].GetType() == Texture::Type::Cubemap)
    {
        BindDraw();
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_color_attachments[index].GetID(), 0);
    }
}

void FrameBufferObject::BindDepthCubemapFace(unsigned int face) const
{
    if (m_depth_stencil_attachment.has_value() && m_depth_stencil_attachment->GetType() == Texture::Type::Cubemap)
    {
        Bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_depth_stencil_attachment->GetID(), 0);
    }
}

void FrameBufferObject::BindDepthCubemapFaceRead(unsigned int face) const
{
    if (m_depth_stencil_attachment.has_value() && m_depth_stencil_attachment->GetType() == Texture::Type::Cubemap)
    {
        BindRead();
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_depth_stencil_attachment->GetID(), 0);
    }
}

void FrameBufferObject::BindDepthCubemapFaceDraw(unsigned int face) const
{
    if (m_depth_stencil_attachment.has_value() && m_depth_stencil_attachment->GetType() == Texture::Type::Cubemap)
    {
        BindDraw();
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, m_depth_stencil_attachment->GetID(), 0);
    }
}

void FrameBufferObject::BindDepthCubemapArray(unsigned int cubemap_index, unsigned int face) const
{
    if (m_depth_stencil_attachment.has_value() && m_depth_stencil_attachment->GetType() == Texture::Type::CubemapArray)
    {
        Bind();
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth_stencil_attachment->GetID(), 0, cubemap_index * 6 + face);
    }
}


void FrameBufferObject::BindColorTexture(size_t index, unsigned int unit)
{
    if (index < m_color_attachments.size())
    {
        m_color_attachments[index].Bind(unit);
    }
}

void FrameBufferObject::BindDepthTexture(unsigned int unit)
{
    if (m_depth_stencil_attachment.has_value())
    {
        m_depth_stencil_attachment->Bind(unit);
    }
}

std::vector<size_t> FrameBufferObject::GetCubemapColorAttachmentIndices() const
{
    std::vector<size_t> indices;
    for (size_t i = 0; i < m_color_attachments.size(); ++i)
    {
        if (m_color_attachments[i].GetType() == Texture::Type::Cubemap)
            indices.push_back(i);
    }
    return indices;
}
}// namespace Aurora