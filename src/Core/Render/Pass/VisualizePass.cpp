// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/Pass/VisualizePass.h"
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"
#include "Core/Render/RenderGraph/RenderGraphContext.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/SceneManager.h"
#include "glWrapper/RenderEventInfo.h"
#include "glWrapper/FrameBufferObject.h"
#include "Runtime/Scene/Camera.h"
#include "Runtime/GlobalContext.h"

namespace Aurora
{

namespace
{
struct VisualizeParameter : PassParameterBase
{
    RenderGraphTextureHandle color_in;
    RenderGraphTextureHandle color_out;
    RenderGraphTextureHandle depth;
};
} // namespace

bool VisualizePass::Init()
{
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
    m_spatial_hierarchy_vao = std::make_shared<VertexArray>();
    m_spatial_hierarchy_vao->Bind();
    m_spatial_hierarchy_vbo->Bind();
    m_spatial_hierarchy_vao->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0});
    m_spatial_hierarchy_vao->Unbind();

    return true;
}

RenderGraphTextureHandle VisualizePass::Render(RenderGraphBuilder& graph_builder,
                                               RenderGraphTextureHandle color,
                                               RenderGraphTextureHandle depth)
{
    ShaderProgram* shader = m_spatial_hierarchy_shader_program.get();
    if (shader == nullptr) return color;

    VisualizeParameter* param = graph_builder.AllocateParameter<VisualizeParameter>();
    param->color_in = color;
    param->color_out = graph_builder.CreateAliasedTexture("SceneColor.Visualize", color);
    param->depth = depth;

    graph_builder.AddPass(
        "Visualize",
        [param](RenderGraphPass& pass)
        {
            pass.Consume(param->color_in);
            pass.Produce(param->color_out);
            pass.Consume(param->depth);
        },
        [param, this, shader](RenderGraphContext& context)
        {
            if (!GlobalContext::GetInstance().draw_spatial_hierarchy) return;

            FrameBufferObject& fbo = context.GetFrameBuffer({param->color_out}, param->depth);
            fbo.Bind();

            RenderState render_state;
            context.ApplyRenderState(render_state);

            // How to transfer data from cpu to gpu efficiently?
            // const auto vertices = SceneManager::GetInstance().GetOctreeVertices();
            const auto vertices = SceneManager::GetInstance().GetBVHVertices();
            m_spatial_hierarchy_vbo->LoadData(vertices, vertices.size());
            shader->Bind();
            m_spatial_hierarchy_vao->Bind();
            shader->SetUniform("uViewProjection", MainCamera::GetInstance().GetProjectionMatrix() * MainCamera::GetInstance().GetViewMatrix());
            shader->SetUniform("uColor", glm::vec3(1.0f, 0.0f, 0.0f));
            glLineWidth(2.0f);
            glDrawArrays(GL_LINES, 0, vertices.size());
            m_spatial_hierarchy_vao->Unbind();
            shader->Unbind();
        });

    // Depth is only read here, so it is passed through unchanged.
    return param->color_out;
}
} // namespace Aurora

