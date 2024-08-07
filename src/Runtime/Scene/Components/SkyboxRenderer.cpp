// std include

// thirdparty include
#include "thirdparty/spdlog/include/spdlog/spdlog.h"
// Aurora include
#include "Runtime/Scene/Components/SkyboxRenderer.h"
#include "Core/Render/Material/SkyboxRenderMaterial.h"

namespace Aurora
{

void SkyboxRenderer::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("SkyboxRendererComponent");

    if (m_mesh)
    {
        auto mesh_node = node->InsertNewChildElement("Temp");
        m_mesh->Serialize(mesh_node);
    }

    if (m_material)
    {
        auto material_node = node->InsertNewChildElement("Temp");
        m_material->Serialize(material_node);
    }
}

void SkyboxRenderer::Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner)
{
    Component::Init(owner);
    m_mesh = nullptr;
    m_material = nullptr;

    auto p_child = node->FirstChildElement();
    while (p_child)
    {
        if (!strcmp(p_child->Name(), "Mesh"))
        {
            m_mesh = std::make_shared<Mesh>();
            m_mesh->Deserialize(p_child, owner);
        }
        else if (!strcmp(p_child->Name(), "Material"))
        {
            auto type = p_child->Attribute("Type");
            if (!strcmp(type, "Skybox"))
            {
                if (m_mesh == nullptr)
                {
                    spdlog::error("SkyboxRendererComponent must have a mesh before material.");
                    return;
                }
                m_material = std::make_shared<SkyboxRenderMaterial>(m_mesh);
                m_material->Deserialize(p_child, owner);
            }
            else spdlog::error("Unknown material type {} in SkyboxRendererComponent.", type);
        }
        else
        {
            spdlog::error("Unknown element {} in SkyboxRendererComponent.", p_child->Name());
        }
        p_child = p_child->NextSiblingElement();
    }
}

void SkyboxRenderer::Update()
{
}

void SkyboxRenderer::Load(std::array<std::string, 6>&& paths)
{
    m_mesh = std::make_shared<Mesh>();
    m_mesh->Init(m_scene_object.lock());
    m_mesh->Load("assets/models/sphere.obj");

    m_material = std::make_shared<SkyboxRenderMaterial>(m_mesh);
    m_material->Init(m_scene_object.lock());
    m_material->Load(std::move(paths));
}
} // namespace Aurora