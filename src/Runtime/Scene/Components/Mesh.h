#pragma once
// std include
#include <string>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
#include "thirdparty/assimp/include/assimp/scene.h"
// Aurora include
#include "Runtime/Scene/Components/Component.h"
#include "Runtime/Scene/Components/Submesh.h"
#include "Utility/Reflection/ReflectionRegister.h"

namespace Aurora
{

class Mesh : public Component
{
    friend class MeshRenderMaterial;
    friend class SkyboxRenderMaterial;
    friend class MeshPhongPass;
    friend class MeshOutlinePass;
    friend class SkyboxPass;
public:
    Mesh() : Component("Mesh") { }
    ~Mesh() = default;

    bool Load(const std::string& file_path);

    void Serialize(tinyxml2::XMLElement* node) override;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    void Update() override;

private:
    void ProcessNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform);
    SubMesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform);
    std::vector<TextureID> LoadMaterialTextures(aiMaterial* material, aiTextureType type, const std::string& base_path);
    std::string ConvertaiTextureTypeToString(aiTextureType type) const;

private:
    std::vector<SubMesh> m_submeshes;
    std::unordered_map<std::string, TextureID> m_texturePath_to_id;

    REFLECTABLE_DECLARE(Mesh, m_path)
    std::string m_path;
};

} // namespace Aurora
