#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Runtime/Scene/Components/Mesh.h"
#include "Core/Render/Material/MeshRenderMaterial.h"

namespace Aurora
{

class MeshRenderer : public Component
{
    friend class Octree;
    friend class BoundingVolumeHierarchy;
public:
    MeshRenderer() : Component("MeshRenderer") { }
    ~MeshRenderer() = default;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    void Update() override;

    bool LoadMesh(const std::string& path);

    inline std::shared_ptr<MeshRenderMaterial> GetRenderMaterial() const { return m_material; }
    const AxisAlignedBoundingBox GetAABB() const { return m_mesh->GetAABB(); }

private:
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<MeshRenderMaterial> m_material;
};
} // namespace Aurora