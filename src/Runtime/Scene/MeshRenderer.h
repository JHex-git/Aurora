#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Runtime/Scene/Mesh.h"
#include "Core/Render/Material/RenderMaterial.h"

namespace Aurora
{

class MeshRenderer : public Component
{
public:
    MeshRenderer() : Component() { m_class_name = "MeshRenderer"; }
    ~MeshRenderer() = default;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node) override;

    void Update() override;

    void LoadMesh(const std::string& path);

private:
    std::shared_ptr<Mesh> m_mesh;
    std::unique_ptr<RenderMaterial> m_material;
};
} // namespace Aurora