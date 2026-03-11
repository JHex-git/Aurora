// std include
#include <cstring>

// thirdparty include

// Aurora include
#include "Runtime/Scene/RenderSettings.h"

namespace Aurora
{

const char* RenderSettings::RenderPathToString(RenderPath path)
{
    switch (path)
    {
        case RenderPath::Forward: return "Forward";
        case RenderPath::Deferred: return "Deferred";
        default: return "Forward";
    }
}

RenderSettings::RenderPath RenderSettings::RenderPathFromString(const char* value)
{
    if (value && strcmp(value, "Deferred") == 0)
        return RenderPath::Deferred;
    return RenderPath::Forward;
}

void RenderSettings::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("RenderSettings");
    node->SetAttribute("RenderPath", RenderPathToString(m_render_path));
    node->SetAttribute("DirectionalCascadeCount", m_directional_cascade_count);
    node->SetAttribute("DirectionalShadowMapSize", m_directional_shadow_map_size);
    node->SetAttribute("DirectionalSplitLambda", m_directional_split_lambda);
    node->SetAttribute("DirectionalPaddingXY", m_directional_padding_xy);
    node->SetAttribute("DirectionalPaddingZ", m_directional_padding_z);
}

void RenderSettings::Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> /*owner*/)
{
    const char* render_path_str = nullptr;
    int cascade_count = m_directional_cascade_count;
    int shadow_map_size = m_directional_shadow_map_size;
    float split_lambda = m_directional_split_lambda;
    float padding_xy = m_directional_padding_xy;
    float padding_z = m_directional_padding_z;

    node->QueryStringAttribute("RenderPath", &render_path_str);
    node->QueryIntAttribute("DirectionalCascadeCount", &cascade_count);
    node->QueryIntAttribute("DirectionalShadowMapSize", &shadow_map_size);
    node->QueryFloatAttribute("DirectionalSplitLambda", &split_lambda);
    node->QueryFloatAttribute("DirectionalPaddingXY", &padding_xy);
    node->QueryFloatAttribute("DirectionalPaddingZ", &padding_z);

    if (render_path_str)
        m_render_path = RenderPathFromString(render_path_str);
    SetDirectionalCascadeCount(cascade_count);
    SetDirectionalShadowMapSize(shadow_map_size);
    SetDirectionalSplitLambda(split_lambda);
    SetDirectionalPaddingXY(padding_xy);
    SetDirectionalPaddingZ(padding_z);
}
} // namespace Aurora
