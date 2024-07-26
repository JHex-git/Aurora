#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Runtime/Scene/Components/Mesh.h"
#include "Core/Render/Material/SkyboxRenderMaterial.h"

namespace Aurora
{

class SkyboxRenderer : public Component
{
public:
    SkyboxRenderer() : Component("SkyboxRenderer") { }
    ~SkyboxRenderer() = default;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    void Update() override;

    void Load(std::array<std::string, 6>&& paths);

private:
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<SkyboxRenderMaterial> m_material;
};
} // namespace Aurora