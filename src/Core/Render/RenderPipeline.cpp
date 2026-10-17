// std include

// thirdparty include

// Aurora include
#include "Core/Render/RenderPipeline.h"
#include "Runtime/Scene/SceneManager.h"
#include "Runtime/Scene/RenderSettings.h"
#include "Runtime/Scene/Components/MeshRenderer.h"
#include "glWrapper/Utils.h"
#include "glWrapper/RenderEventInfo.h"
#include "Core/Render/RenderResourcePool.h"
#include "Core/Render/RenderGraph/RenderGraphBuilder.h"

namespace Aurora
{

RenderPipeline::RenderPipeline(std::array<int, 2> render_size) :
        m_render_size(render_size)
{
    auto final_color = TextureFactory::Create({.width = render_size[0],
                                               .height = render_size[1],
                                               .internal_format = GL_RGBA8,
                                               .format = GL_RGBA,
                                               .type = GL_UNSIGNED_BYTE});
    if (final_color.has_value())
    {
        m_final_color_texture = std::make_unique<Texture>(std::move(*final_color));
    }
    m_resource_pool = std::make_unique<RenderResourcePool>();
}

RenderPipeline::~RenderPipeline() = default;

bool RenderPipeline::Init()
{
    m_shadow_pass = std::make_unique<ShadowPass>();
    m_mesh_phong_pass = std::make_unique<ForwardRenderPass>();
    m_deferred_pass = std::make_unique<DeferredRenderPass>();
    m_mesh_outline_pass = std::make_unique<MeshOutlinePass>();
    m_skybox_pass = std::make_unique<SkyboxPass>();
    m_tonemap_pass = std::make_unique<TonemapPass>();
    m_gizmos_pass = std::make_unique<GizmosPass>();
    m_visualize_pass = std::make_unique<VisualizePass>();
    return m_shadow_pass->Init() &&
           m_mesh_phong_pass->Init() &&
           m_deferred_pass->Init() &&
           m_mesh_outline_pass->Init() &&
           m_skybox_pass->Init() &&
           m_tonemap_pass->Init() &&
           m_gizmos_pass->Init() &&
           m_visualize_pass->Init();
}

void RenderPipeline::Render(ContextState& context_state)
{
    auto scene = SceneManager::GetInstance().GetScene();
    if (!scene) return;

    RenderGraphBuilder graph_builder(m_resource_pool.get());
    const auto& settings = scene->GetRenderSettings();
    const std::array<int, 2>& size = m_render_size;

    // Scene targets. The depth carries stencil so the outline pass can use it.
    auto color = graph_builder.CreateTexture(
        "SceneColor",
        {.width = size[0], .height = size[1], .internal_format = GL_RGBA16F, .format = GL_RGBA, .type = GL_FLOAT});
    auto depth = graph_builder.CreateTexture(
        "SceneDepth",
        {.width = size[0], .height = size[1], .internal_format = GL_DEPTH24_STENCIL8, .format = GL_DEPTH_STENCIL, .type = GL_UNSIGNED_INT_24_8});

    SCOPED_RENDER_EVENT("Scene Rendering");

    const ShadowPass::Result shadows = m_shadow_pass->Render(graph_builder);

    if (settings.GetRenderPath() == RenderSettings::RenderPath::Deferred)
        m_deferred_pass->Render(graph_builder, color, depth, shadows, size);
    else
        m_mesh_phong_pass->Render(graph_builder, color, depth, shadows, size);

    color = m_visualize_pass->Render(graph_builder, color, depth);
    color = m_skybox_pass->Render(graph_builder, color, depth);

    auto final_color = graph_builder.RegisterExternalTexture("BackBuffer", m_final_color_texture.get());
    color = m_tonemap_pass->Render(graph_builder, color, final_color, size);

    auto selected_scene_object = scene->GetSelectedSceneObject();
    if (selected_scene_object != nullptr)
    {
        if (auto selected_mesh_renderer = selected_scene_object->TryGetComponent<MeshRenderer>())
        {
            m_mesh_outline_pass->SetMeshRenderMaterial(selected_mesh_renderer->GetRenderMaterial());
            RenderGraphTextureHandle depth_after_outline;
            color = m_mesh_outline_pass->Render(graph_builder, color, depth, depth_after_outline, size);
            depth = depth_after_outline;
        }
        m_gizmos_pass->SetSelectedTransform(selected_scene_object->GetTransform());
    }

    color = m_gizmos_pass->Render(graph_builder, color, depth);

    graph_builder.Compile();
    graph_builder.Execute(context_state, size);
}

int RenderPipeline::GetRenderTargetID() const
{
    return m_final_color_texture != nullptr ? m_final_color_texture->GetID() : 0;
}

void RenderPipeline::SetSkyboxRenderMaterial(std::shared_ptr<SkyboxRenderMaterial> skybox_render_material)
{
    m_skybox_pass->SetSkyboxRenderMaterial(skybox_render_material);
}
} // namespace Aurora
