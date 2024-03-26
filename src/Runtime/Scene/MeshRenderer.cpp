// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Scene/MeshRenderer.h"
#include "Core/Render/Material/MeshRenderMaterial.h"

namespace Aurora
{

void MeshRenderer::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("MeshRendererComponent");

    if (m_mesh)
    {
        auto mesh_node = node->InsertNewChildElement(nullptr);
        m_mesh->Serialize(mesh_node);
    }

    if (m_material)
    {
        auto material_node = node->InsertNewChildElement(nullptr);
        m_material->Serialize(material_node);
    }
}

void MeshRenderer::Deserialize(const tinyxml2::XMLElement *node)
{
    m_mesh = nullptr;
    m_material = nullptr;

    auto p_child = node->FirstChildElement();
    while (p_child)
    {
        if (!strcmp(p_child->Name(), "Mesh"))
        {
            m_mesh = std::make_shared<Mesh>();
            m_mesh->Deserialize(p_child);
        }
        else if (!strcmp(p_child->Name(), "Material"))
        {
            auto type = p_child->Attribute("Type");
            if (!strcmp(type, "Default"))
            {
                if (m_mesh == nullptr)
                {
                    spdlog::error("MeshRendererComponent must have a mesh before material.");
                    return;
                }
                m_material = std::make_unique<MeshRenderMaterial>(m_mesh);
                m_material->Deserialize(p_child);
            }
            else spdlog::error("Unknown material type {} in MeshRendererComponent.", type);
        }
        else
        {
            spdlog::error("Unknown element {} in MeshRendererComponent.", p_child->Name());
        }
        p_child = p_child->NextSiblingElement();
    }
}

void MeshRenderer::Update()
{
}

void MeshRenderer::LoadMesh(const std::string& path)
{
    m_mesh = std::make_shared<Mesh>();
    m_mesh->Load(path);

    m_material = std::make_unique<MeshRenderMaterial>(m_mesh);
    m_material->Init();
}
} // namespace Aurora