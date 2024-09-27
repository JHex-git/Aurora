// std include
#include <filesystem>
// thirdparty include
#include "thirdparty/assimp/include/assimp/Importer.hpp"
#include "thirdparty/assimp/include/assimp/scene.h"
#include "thirdparty/assimp/include/assimp/postprocess.h"
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Runtime/Scene/Components/Mesh.h"
#include "Runtime/Scene/Components/SubMesh.h"
#include "Runtime/Scene/SceneObjects/SceneObject.h"
#include "glWrapper/Texture.h"
#include "ExternalHelper/AssimpHelper.h"
#include "Utility/FileSystem.h"
#include "Runtime/Scene/TextureManager.h"
#include "Core/Render/Material/MeshRenderMaterial.h"
#include "Runtime/Scene/SceneManager.h"

namespace Aurora
{
REFLECTABLE_IMPL(Mesh, m_path, std::string)

Mesh::~Mesh()
{
    SceneManager::GetInstance().UnregisterMesh(m_id);
}

void Mesh::Serialize(tinyxml2::XMLElement* node)
{
    node->SetName("Mesh");
    node->SetAttribute("Path", m_path.c_str());
}

void Mesh::Deserialize(const tinyxml2::XMLElement* node, std::shared_ptr<SceneObject> owner)
{
    Component::Init(owner);
    Load(node->Attribute("Path"));
}

void Mesh::Update()
{
}

bool Mesh::Load(const std::string& file_path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(FileSystem::GetFullPath(file_path), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        spdlog::error("Failed to load mesh: {}", importer.GetErrorString());
        return false;
    }

    m_path = file_path;
    ProcessNode(scene->mRootNode, scene, glm::mat4(1.0f));
    spdlog::info("Mesh {} loaded successfully.", file_path);
    
    m_id = SceneManager::GetInstance().RegisterMesh(shared_from_this()); 
    return true;
}

const AxisAlignedBoundingBox Mesh::GetAABB() const
{
    if (auto scene_object = m_scene_object.lock())
    {
        auto model_transform = scene_object->GetTransform();
        return glm::mat4(*model_transform) * m_local_aabb;
    }
    else return {};
}

void Mesh::ProcessNode(const aiNode* node, const aiScene* scene, const glm::mat4& parentTransform)
{
    auto node_transform = node->mTransformation;
    glm::mat4 transform = parentTransform * AssimpHelper::MatrixToGLM(node_transform);
    // process all the node’s meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        m_submeshes.push_back(ProcessMesh(mesh, scene, transform));

        auto min_vertex = glm::vec3(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z);
        auto max_vertex = glm::vec3(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z);
        m_local_aabb |= transform * AxisAlignedBoundingBox(min_vertex, max_vertex);
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, transform);
    }
}

SubMesh Mesh::ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<TextureID> textures;
    auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
    // process vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        glm::vec3 position, normal;
        glm::vec2 tex_coord;
        glm::vec4 color;
        if (mesh->HasPositions())
        {
            position = transform * glm::vec4(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z), 1.0f);
        }

        if (mesh->HasNormals())
        {
            normal = glm::normalize(normalMatrix * glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
        }

        if (mesh->HasTextureCoords(0))
        {
            tex_coord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        
        if (mesh->HasVertexColors(0))
        {
            color = glm::vec4(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);
        }

        vertices.emplace_back(std::move(position), std::move(normal), std::move(tex_coord), std::move(color));
    }
    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // process each face’s indices
        for(unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }
    // process material
    aiColor3D color;
    if (mesh->mMaterialIndex >= 0)
    {
        std::filesystem::path base_path(m_path);
        auto base_path_str = base_path.remove_filename().string();
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        // diffuse maps
        std::vector<TextureID> albedoMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, base_path_str);
        textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());
        // specular maps
        std::vector<TextureID> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, base_path_str);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // normal maps
        std::vector<TextureID> normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, base_path_str);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // height maps
        std::vector<TextureID> heightMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, base_path_str);
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        // metallic maps
        std::vector<TextureID> metallicMaps = LoadMaterialTextures(material, aiTextureType_METALNESS, base_path_str);
        textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());
        // // roughness maps
        // std::vector<TextureID> roughnessMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, base_path_str);
        // textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());
        // emissive maps
        std::vector<TextureID> emissiveMaps = LoadMaterialTextures(material, aiTextureType_EMISSIVE, base_path_str);
        textures.insert(textures.end(), emissiveMaps.begin(), emissiveMaps.end());
        // displacement maps
        std::vector<TextureID> displacementMaps = LoadMaterialTextures(material, aiTextureType_DISPLACEMENT, base_path_str);
        textures.insert(textures.end(), displacementMaps.begin(), displacementMaps.end());
        // ao maps
        std::vector<TextureID> aoMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, base_path_str);
        textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
    }

    if (!textures.empty()) m_has_textures = true;
    return SubMesh(color, std::move(vertices), std::move(indices), std::move(textures));
}

std::vector<TextureID> Mesh::LoadMaterialTextures(aiMaterial* material, aiTextureType type, const std::string& base_path)
{
    std::vector<TextureID> texture_ids;
    for(unsigned int i = 0; i < material->GetTextureCount(type); i++)
    {
        aiString path;
        material->GetTexture(type, i, &path);
        auto type_str = std::string(aiTextureTypeToString(type));
        auto full_path = base_path + path.C_Str();
        if (auto it = m_texturePath_to_id.find(full_path); it != m_texturePath_to_id.end())
        {
            texture_ids.push_back(it->second);
            continue;
        }
        if (auto texture = TextureBuilder().WithWrapS(TextureBuilder::WrapType::ClampToEdge).WithWrapT(TextureBuilder::WrapType::ClampToEdge).MakeTexture2D(full_path))
        {
            m_texturePath_to_id.insert({full_path, texture->GetID()});
            texture_ids.push_back(texture->GetID());
            TextureManager::GetInstance().m_surface_textures.insert({texture->GetID(), SurfaceTexture(std::move(*texture), std::move(type_str))});
        }
    }
    return texture_ids;
}

std::string Mesh::ConvertaiTextureTypeToString(aiTextureType type) const
{
    if (type == aiTextureType_HEIGHT) return "Normal";
    return aiTextureTypeToString(type);
}
} // namespace Aurora