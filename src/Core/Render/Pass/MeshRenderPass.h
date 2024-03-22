#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "glWrapper/Shader.h"
#include "Core/Render/Pass/RenderPass.h"
#include "Runtime/Scene/Mesh.h"
#include "glWrapper/VertexBuffer.h"
#include "glWrapper/ElementBuffer.h"

namespace Aurora
{

class MeshRenderPass : public RenderPass
{
public:
    MeshRenderPass(std::shared_ptr<Mesh> mesh) : m_mesh(mesh) { }
    ~MeshRenderPass() = default;

    bool Init() override;
    void Render() override;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node) override;

private:
    std::shared_ptr<ShaderProgram> m_shader_program;

    std::shared_ptr<Mesh> m_mesh;
    std::vector<std::shared_ptr<VertexBuffer>> m_vbos;
    std::vector<std::shared_ptr<ElementBuffer>> m_ebos;
};

} // namespace Aurora
