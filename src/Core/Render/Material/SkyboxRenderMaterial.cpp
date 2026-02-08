// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
#include "thirdparty/opengl/glm/glm/gtc/matrix_transform.hpp"
// Aurora include
#include "Core/Render/Material/SkyboxRenderMaterial.h"
#include "Core/Render/RenderSystem.h"

namespace Aurora
{

void SkyboxRenderMaterial::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("Material");
    node->SetAttribute("Type", "Skybox");

    for (size_t i = 0; i < m_cubemap_paths.size(); ++i)
    {
        auto path_node = node->InsertNewChildElement("Path");
        path_node->SetText(m_cubemap_paths[i].c_str());
    }
}

void SkyboxRenderMaterial::Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner)
{
    Init(owner);

    // parse the paths in the xml file
    auto p_child = node->FirstChildElement();
    for (size_t i = 0; i < 6; ++i)
    {
        if (p_child)
        {
            m_cubemap_paths[i] = p_child->GetText();
            p_child = p_child->NextSiblingElement();
        }
        else
        {
            spdlog::error("Skybox must have 6 textures.");
            return;
        }
    }

    auto cubemap_path = m_cubemap_paths;
    Load(std::move(cubemap_path));
}

bool SkyboxRenderMaterial::Init(std::shared_ptr<SceneObject> owner)
{
    RenderMaterial::Init(owner);

    // TODO: normal and texcoords of Vertex are not needed for skybox
    m_vbos.resize(m_mesh->m_submeshes.size());
    for (size_t i = 0; i < m_mesh->m_submeshes.size(); ++i)
    {
        m_vbos[i] = std::make_shared<VertexBuffer>();
        m_vbos[i]->LoadData(m_mesh->m_submeshes[i].m_vertices, m_mesh->m_submeshes[i].m_vertices.size());
    }
    
    m_ebos.resize(m_mesh->m_submeshes.size());
    for (size_t i = 0; i < m_mesh->m_submeshes.size(); ++i)
    {
        m_ebos[i] = std::make_shared<ElementBuffer>();
        m_ebos[i]->LoadData(m_mesh->m_submeshes[i].m_indices, m_mesh->m_submeshes[i].m_indices.size());
    }
    
    m_vaos.resize(m_mesh->m_submeshes.size());
    for (size_t i = 0; i < m_mesh->m_submeshes.size(); ++i)
    {
        m_vaos[i] = std::make_shared<VertexArray>();
        m_vaos[i]->Bind();
        m_vbos[i]->Bind();
        m_ebos[i]->Bind();
        m_vaos[i]->SetAttribPointer(VertexAttribPointer{0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)});
        m_vaos[i]->Unbind();
    }
    RenderSystem::GetInstance().SetSkyboxRenderMaterial(shared_from_this());
    return true;
}

void SkyboxRenderMaterial::Load(std::array<std::string, 6>&& paths)
{
    m_cubemap_texture = TextureBuilder().WithWrapS(TextureBuilder::WrapType::ClampToEdge)
                                        .WithWrapT(TextureBuilder::WrapType::ClampToEdge)
                                        .WithWrapR(TextureBuilder::WrapType::ClampToEdge).MakeTextureCubeMap(paths);
    m_cubemap_paths = std::move(paths);
}
} // namespace Aurora