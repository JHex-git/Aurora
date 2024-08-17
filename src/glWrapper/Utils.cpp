// std include

// thirdparty include
// Aurora include
#include "glWrapper/Utils.h"
#include "glWrapper/FrameBufferObject.h"

namespace Aurora
{

void Blit(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLbitfield mask, GLenum filter)
{
    src->BindRead();
    dst->BindDraw();
    const auto&& [src_width, src_height] = src->GetSize();
    const auto&& [dst_width, dst_height] = dst->GetSize();
    glBlitFramebuffer(0, 0, src_width, src_height, 0, 0, dst_width, dst_height, mask, filter);
    src->UnbindRead();
    dst->UnbindDraw();
}

void BlitColor(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLenum filter)
{
    Blit(src, dst, GL_COLOR_BUFFER_BIT, filter);
}

void BlitDepth(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLenum filter)
{
    Blit(src, dst, GL_DEPTH_BUFFER_BIT, filter);
}

void BlitStencil(std::shared_ptr<FrameBufferObject> src, std::shared_ptr<FrameBufferObject> dst, GLenum filter)
{
    Blit(src, dst, GL_STENCIL_BUFFER_BIT, filter);
}

void DrawQuad(TextureID texture)
{
    static GLuint quad_vao = 0;
    static GLuint quad_vbo = 0;
    if (quad_vao == 0)
    {
        float quad_vertices[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        };

        glGenVertexArrays(1, &quad_vao);
        glBindVertexArray(quad_vao);

        glGenBuffers(1, &quad_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
}
} // namespace Aurora