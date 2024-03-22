#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "glWrapper/Shader.h"
#include "Core/Render/Pass/RenderPass.h"

namespace Aurora
{

class PhongRenderPass : public RenderPass
{
public:
    PhongRenderPass() = default;
    ~PhongRenderPass() = default;

    bool Init() override;
    void Render() override;

private:
    std::shared_ptr<ShaderProgram> m_shader_program;
};

} // namespace Aurora
