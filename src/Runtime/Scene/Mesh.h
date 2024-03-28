#pragma once
// std include
#include <string>
// thirdparty include
#include "thirdparty/opengl/glm/glm/glm.hpp"
#include "thirdparty/assimp/include/assimp/scene.h"
// Aurora include
#include "Runtime/Scene/TextureInfo.h"
#include "Runtime/Scene/Component.h"
#include "Runtime/Scene/Submesh.h"
#include "Utility/Reflection/ReflectionRegister.h"

namespace Aurora
{
class Texture;

class Mesh : public Component
{
    friend class MeshRenderMaterial;
    friend class MeshPhongPass;
public:
    Mesh() : Component() { m_class_name = "Mesh"; }
    ~Mesh() = default;

    bool Load(const std::string& file_path);

    void Serialize(tinyxml2::XMLElement* node) override;
    void Deserialize(const tinyxml2::XMLElement *node) override;

    void Update() override;

private:
    void ProcessNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform);
    SubMesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform) const;
    std::vector<TextureInfo> LoadMaterialTextures(aiMaterial* material, aiTextureType type, const std::string& base_path) const;
    std::string ConvertaiTextureTypeToString(aiTextureType type) const;

    
private:
    std::vector<SubMesh> m_submeshes;

    REFLECTABLE_DECLARE(Mesh, m_path)
    std::string m_path;
    
};

} // namespace Aurora
