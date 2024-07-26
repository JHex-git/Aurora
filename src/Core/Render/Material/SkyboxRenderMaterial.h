#pragma once
// std include
#include <memory>
// thirdparty include

// Aurora include
#include "Core/Render/Material/RenderMaterial.h"
#include "glWrapper/Texture.h"
#include "Runtime/Scene/Components/Mesh.h"
#include "glWrapper/VertexBuffer.h"
#include "glWrapper/ElementBuffer.h"

namespace Aurora
{
class SkyboxRenderMaterial : public RenderMaterial, public std::enable_shared_from_this<SkyboxRenderMaterial>
{
    friend class SkyboxPass;
public:
    SkyboxRenderMaterial(std::shared_ptr<Mesh> mesh) : RenderMaterial("SkyboxRenderMaterial"), m_mesh(mesh) { }
    ~SkyboxRenderMaterial() = default;

    bool Init(std::shared_ptr<SceneObject> owner) override;
    void Load(std::array<std::string, 6>&& paths);

    void Serialize(tinyxml2::XMLElement *node) override final;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override final;

    std::optional<Texture>& GetCubemapTexture() { return m_cubemap_texture; }

private:
    std::optional<Texture> m_cubemap_texture;
    std::array<std::string, 6> m_cubemap_paths;

    std::shared_ptr<Mesh> m_mesh;
    std::vector<std::shared_ptr<VertexBuffer>> m_vbos;
    std::vector<std::shared_ptr<ElementBuffer>> m_ebos;
};
} // namespace Aurora