// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/Pass/VisualizePass.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/SceneManager.h"
#include "glWrapper/RenderEventInfo.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/GlobalContext.h"

namespace Aurora
{

bool VisualizePass::Init(const std::array<int, 2>& viewport_size)
{
    if (!RenderPass::Init(viewport_size)) return false;
    
    auto fbo = FrameBufferObjectBuilder(viewport_size[0], viewport_size[1])
                                        .AddColorAttachment({.internal_format = GL_RGBA})
                                        .EnableDepthAttachment({}).Create();
    if (!fbo.has_value()) return false;
    m_fbo = std::make_shared<FrameBufferObject>(std::move(fbo.value()));

    // aabb visualize shader program
    {
        std::vector<Shader> shaders;
        shaders.emplace_back(ShaderType::VertexShader);
        auto vert_path = FileSystem::GetFullPath("shaders/position_bypass.vert");
        if (!shaders[0].Load(vert_path))
        {
            spdlog::error("Failed to load vertex shader {}", vert_path);
            return false;
        }
        shaders.emplace_back(ShaderType::GeometryShader);
        auto geom_path = FileSystem::GetFullPath("shaders/aabb.geom");
        if (!shaders[1].Load(geom_path))
        {
            spdlog::error("Failed to load geometry shader {}", geom_path);
            return false;
        }
        shaders.emplace_back(ShaderType::FragmentShader);
        auto frag_path = FileSystem::GetFullPath("shaders/pure.frag");
        if (!shaders[2].Load(frag_path))
        {
            spdlog::error("Failed to load fragment shader {}", frag_path);
            return false;
        }
        m_spatial_hierarchy_shader_program = std::make_unique<ShaderProgram>();
        if (!m_spatial_hierarchy_shader_program->Load(shaders))
        {
            spdlog::error("Failed to load shader program");
            return false;
        }
    }
    m_spatial_hierarchy_vbo = std::make_shared<VertexBuffer>();

    return true;
}

void VisualizePass::Render()
{
    m_fbo->Bind();
    
    if (GlobalContext::GetInstance().draw_spatial_hierarchy)
    {
        SCOPED_RENDER_EVENT("Spatial Hierarchy Visualization");
        // How to transfer data from cpu to gpu efficiently?
        // const auto vertices = SceneManager::GetInstance().GetOctreeVertices();
        const auto vertices = SceneManager::GetInstance().GetBVHVertices();
        m_spatial_hierarchy_vbo->LoadData(vertices, vertices.size());
        m_spatial_hierarchy_vbo->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0});
        m_spatial_hierarchy_shader_program->Bind();
        m_spatial_hierarchy_vbo->Bind();
        m_spatial_hierarchy_shader_program->SetUniform("uViewProjection", MainCamera::GetInstance().GetProjectionMatrix() * MainCamera::GetInstance().GetViewMatrix());
        m_spatial_hierarchy_shader_program->SetUniform("uColor", glm::vec3(1.0f, 0.0f, 0.0f));
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, vertices.size());

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        glDisable(GL_BLEND);
    }
}
} // namespace Aurora