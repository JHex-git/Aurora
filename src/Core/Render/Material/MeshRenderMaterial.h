#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Material/RenderMaterial.h"
#include "Runtime/Scene/Components/Mesh.h"
#include "glWrapper/VertexBuffer.h"
#include "glWrapper/ElementBuffer.h"
#include "Runtime/Scene/SceneObjects/SceneObject.h"

namespace Aurora
{

using MeshRenderMaterialID = unsigned int;

class MeshRenderMaterial : public RenderMaterial, public std::enable_shared_from_this<MeshRenderMaterial>
{
    friend class ForwardRenderPass;
    friend class MeshOutlinePass;
    friend class SkyboxPass;
    friend class GizmosPass;
    
public:
    MeshRenderMaterial(std::shared_ptr<Mesh> mesh) : RenderMaterial("MeshRenderMaterial"), m_mesh(mesh) { }
    virtual ~MeshRenderMaterial() = default;

    bool Init(std::shared_ptr<SceneObject> owner) override;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    const glm::mat4 GetModelMatrix() const;

    static constexpr MeshRenderMaterialID INVALID_MESH_RENDER_MATERIAL_ID = 0;

private:
    MeshRenderMaterialID m_material_id = MeshRenderMaterial::INVALID_MESH_RENDER_MATERIAL_ID;

    std::shared_ptr<Mesh> m_mesh;
    std::vector<std::shared_ptr<VertexBuffer>> m_vbos;
    std::vector<std::shared_ptr<ElementBuffer>> m_ebos;

};

} // namespace Aurora
