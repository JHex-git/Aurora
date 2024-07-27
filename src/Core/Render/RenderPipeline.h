#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/MeshPhongPass.h"
#include "Core/Render/Pass/MeshOutlinePass.h"
#include "Core/Render/Pass/SkyboxPass.h"
#include "glWrapper/FrameBufferObject.h"

namespace Aurora
{

class RenderPipeline
{
public:
    RenderPipeline(std::array<int, 2> render_size = {1920, 1080}) : 
        m_render_size(render_size), 
        m_fbo(FrameBufferObjectBuilder(m_render_size[0], m_render_size[1]).AddColorAttachment(GL_RGBA).EnableDepthAttachment().Create().value())
    { }
    ~RenderPipeline() = default;

    bool Init();
    void Render();

    void AddMeshRenderMaterial(std::shared_ptr<MeshRenderMaterial> mesh_render_material);
    void SetSkyboxRenderMaterial(std::shared_ptr<SkyboxRenderMaterial> skybox_render_material);

    int GetRenderTargetID() const { return m_fbo.GetColorAttachmentID(0); }

private:
    std::unique_ptr<MeshPhongPass> m_mesh_phong_pass;
    std::unique_ptr<MeshOutlinePass> m_mesh_outline_pass;
    std::unique_ptr<SkyboxPass> m_skybox_pass;

    std::array<int, 2> m_render_size;
    FrameBufferObject m_fbo;
};
} // namespace Aurora