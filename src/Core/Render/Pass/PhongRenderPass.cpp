// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Core/Render/Pass/PhongRenderPass.h"


namespace Aurora
{

bool PhongRenderPass::Init()
{
    std::vector<Shader> shaders;
    shaders.emplace_back(Shader(ShaderType::VertexShader));
    shaders.emplace_back(Shader(ShaderType::FragmentShader));

    if (!shaders[0].Load("assets/shaders/bypass.vert"))
    {
        return false;
    }

    if (!shaders[1].Load("assets/shaders/phong.frag"))
    {
        return false;
    }

    m_shader_program = std::make_shared<ShaderProgram>();
    if (!m_shader_program->Load(shaders))
    {
        return false;
    }

    spdlog::info("PhongRenderPass initialized");
    return true;
}

void PhongRenderPass::Render()
{
    m_shader_program->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    m_shader_program->Unbind();
}
} // namespace Aurora