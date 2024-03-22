#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Runtime/Scene/Mesh.h"
#include "Core/Render/Pass/RenderPass.h"

namespace Aurora
{

class MeshRenderer : public Component
{
public:
    MeshRenderer() = default;
    ~MeshRenderer() = default;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node) override;

    void Update() override;

    void LoadMesh(const std::string& path);

private:
    std::shared_ptr<Mesh> m_mesh;
    std::unique_ptr<RenderPass> m_material;
};
} // namespace Aurora