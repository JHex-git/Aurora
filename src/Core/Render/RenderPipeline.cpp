// std include

// thirdparty include

// Aurora include
#include "Core/Render/RenderPipeline.h"
#include "Runtime/Scene/SceneManager.h"
#include "Runtime/Scene/Components/MeshRenderer.h"
#include "glWrapper/Utils.h"

namespace Aurora
{

RenderPipeline::RenderPipeline(std::array<int, 2> render_size) :
        m_render_size(render_size)
{
    auto fbo = FrameBufferObjectBuilder(m_render_size[0], m_render_size[1])
                                        .AddColorAttachment({.internal_format = GL_RGBA} )
                                        .EnableDepthAttachment({.has_stencil = true}).Create();
    if (!fbo.has_value()) m_fbo = nullptr;
    else m_fbo = std::make_shared<FrameBufferObject>(std::move(fbo.value()));
}

bool RenderPipeline::Init()
{
    m_mesh_phong_pass = std::make_unique<MeshPhongPass>();
    m_mesh_outline_pass = std::make_unique<MeshOutlinePass>();
    m_skybox_pass = std::make_unique<SkyboxPass>();
    m_gizmos_pass = std::make_unique<GizmosPass>();
    return m_mesh_phong_pass->Init(m_render_size) && 
           m_mesh_outline_pass->Init(m_render_size) && 
           m_skybox_pass->Init(m_render_size) && 
           m_gizmos_pass->Init(m_render_size);
}

void RenderPipeline::Render()
{
    m_mesh_phong_pass->Render();
    Blit(m_mesh_phong_pass->GetFrameBuffer(), m_skybox_pass->GetFrameBuffer());
    m_skybox_pass->Render();
    RenderPass* prev_pass = m_skybox_pass.get();

    // Outline should be rendered after all to ensure not be occluded
    // Use stencil buffer to avoid covering the selected mesh
    auto selected_scene_object = SceneManager::GetInstance().GetScene()->GetSelectedSceneObject();
    if (selected_scene_object != nullptr)
    {
        auto selected_mesh_renderer = selected_scene_object->TryGetComponent<MeshRenderer>();
        if (selected_mesh_renderer != nullptr)
        {
            m_mesh_outline_pass->SetMeshRenderMaterial(selected_mesh_renderer->GetRenderMaterial());
            BlitColor(m_skybox_pass->GetFrameBuffer(), m_mesh_outline_pass->GetFrameBuffer());
            m_mesh_outline_pass->Render();
            prev_pass = m_mesh_outline_pass.get();
        }

        m_gizmos_pass->SetSelectedTransform(selected_scene_object->GetTransform());
    }

    Blit(prev_pass->GetFrameBuffer(), m_gizmos_pass->GetFrameBuffer());
    m_gizmos_pass->Render();
    Blit(m_gizmos_pass->GetFrameBuffer(), m_fbo);
}

void RenderPipeline::AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material)
{
    m_mesh_phong_pass->AddMeshRenderMaterial(mesh_render_material);
}

void RenderPipeline::SetSkyboxRenderMaterial(std::shared_ptr<SkyboxRenderMaterial> skybox_render_material)
{
    m_skybox_pass->SetSkyboxRenderMaterial(skybox_render_material);
}
} // namespace Aurora