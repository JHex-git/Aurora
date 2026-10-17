#pragma once
// std include
#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <vector>
// thirdparty include

// Aurora include
#include "glWrapper/Texture.h"

namespace Aurora
{
class FrameBufferObject;

struct FrameBufferKey
{
    std::vector<Texture*> colors;
    Texture* depth = nullptr;
    GLsizei width = 0;
    GLsizei height = 0;

    friend bool operator==(const FrameBufferKey&, const FrameBufferKey&) = default;
};

class FrameBufferObject
{
public:
    static std::optional<FrameBufferObject> Create(GLsizei width,
                                                  GLsizei height,
                                                  const std::vector<Texture*>& color_attachments,
                                                  Texture* depth_attachment = nullptr);

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
    void BindDepthTextureLayer(unsigned int layer) const;

    void BindColorTexture(size_t index, unsigned int unit);
    void BindDepthTexture(unsigned int unit);

    unsigned int GetColorAttachmentNumber() const { return static_cast<unsigned int>(m_color_attachments.size()); }
    TextureID GetColorAttachmentID(size_t index) const { const Texture* texture = GetColorAttachment(index); return texture ? texture->GetID() : 0; }
    TextureID GetDepthAttachmentID() const { return m_depth_attachment ? m_depth_attachment->GetID() : 0; }

    // Returns the attachment, or nullptr when the index is out of range.
    Texture* GetColorAttachment(size_t index) const { return index < m_color_attachments.size() ? m_color_attachments[index] : nullptr; }
    Texture* GetDepthAttachment() const { return m_depth_attachment; }

    std::vector<size_t> GetCubemapColorAttachmentIndices() const;

    unsigned int GetID() const { return m_fboID; }
    const std::array<unsigned int, 2> GetSize() const { return { m_width, m_height }; }

private:
    FrameBufferObject(unsigned int width, unsigned int height);

    void AttachAll(GLenum target) const;

    unsigned int m_fboID = 0;
    unsigned int m_width;
    unsigned int m_height;
    std::vector<Texture*>   m_color_attachments;
    Texture*                m_depth_attachment = nullptr;
    GLenum                  m_depth_attachment_point = GL_DEPTH_ATTACHMENT;
};
} // namespace Aurora

namespace std
{
template<>
struct hash<Aurora::FrameBufferKey>
{
    std::size_t operator()(const Aurora::FrameBufferKey& key) const noexcept
    {
        std::size_t seed = 0;
        const auto combine = [&seed](std::size_t value)
        {
            seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
        };

        // Mix the length first so {a} and {a, nullptr} hash differently.
        combine(key.colors.size());
        for (const Aurora::Texture* texture : key.colors)
        {
            combine(std::hash<const Aurora::Texture*>{}(texture));
        }

        combine(std::hash<const Aurora::Texture*>{}(key.depth));
        combine(std::hash<GLsizei>{}(key.width));
        combine(std::hash<GLsizei>{}(key.height));
        return seed;
    }
};
} // namespace std
