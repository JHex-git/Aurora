#pragma once
// std include
#include <string>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
#include "thirdparty/assimp/include/assimp/scene.h"
// Aurora include
#include "Runtime/Scene/Components/Component.h"
#include "Runtime/Scene/Components/Submesh.h"
#include "Math/AxisAlignedBoundingBox.h"
#include "Utility/Reflection/ReflectionRegister.h"

namespace Aurora
{
class RenderMaterial;

using MeshID = unsigned int;

class Mesh : public Component, public std::enable_shared_from_this<Mesh>
{
    friend class MeshRenderMaterial;
    friend class SkyboxRenderMaterial;
    friend class ForwardRenderPass;
    friend class MeshOutlinePass;
    friend class SkyboxPass;
    friend class GizmosPass;
public:
    Mesh() : Component("Mesh") { }
    virtual ~Mesh();

    bool Load(const std::string& file_path);
    void SetRenderProxy(std::shared_ptr<RenderMaterial> render_proxy) { m_render_proxy = render_proxy; }

    void Serialize(tinyxml2::XMLElement* node) override;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    void Update() override;

    const bool HasTextures() const { return m_has_textures; }
    const AxisAlignedBoundingBox GetAABB() const;

    static constexpr MeshID INVALID_MESH_ID = 0;
private:
    void ProcessNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform);
    SubMesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform);
    std::vector<TextureID> LoadMaterialTextures(aiMaterial* material, aiTextureType type, const std::string& base_path);
    std::string ConvertaiTextureTypeToString(aiTextureType type) const;

private:
    std::vector<SubMesh> m_submeshes;
    std::unordered_map<std::string, TextureID> m_texturePath_to_id;
    std::weak_ptr<RenderMaterial> m_render_proxy;
    MeshID m_id = INVALID_MESH_ID;

    bool m_has_textures = false;
    AxisAlignedBoundingBox m_local_aabb;

    REFLECTABLE_DECLARE(Mesh, m_path)
    std::string m_path;
};

} // namespace Aurora
