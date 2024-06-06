#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "glWrapper/Shader.h"
#include "Core/Render/Material/RenderMaterial.h"
#include "Runtime/Scene/Mesh.h"
#include "glWrapper/VertexBuffer.h"
#include "glWrapper/ElementBuffer.h"
#include "Runtime/Scene/SceneObject.h"

namespace Aurora
{

class MeshRenderMaterial : public RenderMaterial, public std::enable_shared_from_this<MeshRenderMaterial>
{
    friend class MeshPhongPass;
    
public:
    MeshRenderMaterial(std::shared_ptr<Mesh> mesh) : RenderMaterial(), m_mesh(mesh) { m_class_name = "MeshRenderMaterial"; }
    ~MeshRenderMaterial() = default;

    bool Init(std::shared_ptr<SceneObject> owner) override;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    const glm::mat4 GetModelMatrix() const;

private:
    std::shared_ptr<Mesh> m_mesh;
    std::vector<std::shared_ptr<VertexBuffer>> m_vbos;
    std::vector<std::shared_ptr<ElementBuffer>> m_ebos;
};

} // namespace Aurora
