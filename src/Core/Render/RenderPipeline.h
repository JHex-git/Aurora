#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Pass/ForwardRenderPass.h"
#include "Core/Render/Pass/MeshOutlinePass.h"
#include "Core/Render/Pass/SkyboxPass.h"
#include "Core/Render/Pass/GizmosPass.h"
#include "Core/Render/Pass/VisualizePass.h"
#include "glWrapper/FrameBufferObject.h"

namespace Aurora
{

class RenderPipeline
{
public:
    RenderPipeline(std::array<int, 2> render_size = {1920, 1080});
    ~RenderPipeline() = default;

    bool Init();
    void Render();

    void SetSkyboxRenderMaterial(std::shared_ptr<SkyboxRenderMaterial> skybox_render_material);

    int GetRenderTargetID() const { return m_fbo->GetColorAttachmentID(0); }

private:
    std::unique_ptr<ForwardRenderPass> m_mesh_phong_pass;
    std::unique_ptr<MeshOutlinePass> m_mesh_outline_pass;
    std::unique_ptr<SkyboxPass> m_skybox_pass;
    std::unique_ptr<VisualizePass> m_visualize_pass;
    std::unique_ptr<GizmosPass> m_gizmos_pass;

    std::array<int, 2> m_render_size;
    std::shared_ptr<FrameBufferObject> m_fbo;
};
} // namespace Aurora