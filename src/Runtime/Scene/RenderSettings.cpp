// std include

// thirdparty include

// Aurora include
#include "Runtime/Scene/RenderSettings.h"

namespace Aurora
{

void RenderSettings::Serialize(tinyxml2::XMLElement *node)
{
    node->SetName("RenderSettings");
    node->SetAttribute("DirectionalCascadeCount", m_directional_cascade_count);
    node->SetAttribute("DirectionalShadowMapSize", m_directional_shadow_map_size);
    node->SetAttribute("DirectionalSplitLambda", m_directional_split_lambda);
    node->SetAttribute("DirectionalPaddingXY", m_directional_padding_xy);
    node->SetAttribute("DirectionalPaddingZ", m_directional_padding_z);
}

void RenderSettings::Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> /*owner*/)
{
    int cascade_count = m_directional_cascade_count;
    int shadow_map_size = m_directional_shadow_map_size;
    float split_lambda = m_directional_split_lambda;
    float padding_xy = m_directional_padding_xy;
    float padding_z = m_directional_padding_z;

    node->QueryIntAttribute("DirectionalCascadeCount", &cascade_count);
    node->QueryIntAttribute("DirectionalShadowMapSize", &shadow_map_size);
    node->QueryFloatAttribute("DirectionalSplitLambda", &split_lambda);
    node->QueryFloatAttribute("DirectionalPaddingXY", &padding_xy);
    node->QueryFloatAttribute("DirectionalPaddingZ", &padding_z);

    SetDirectionalCascadeCount(cascade_count);
    SetDirectionalShadowMapSize(shadow_map_size);
    SetDirectionalSplitLambda(split_lambda);
    SetDirectionalPaddingXY(padding_xy);
    SetDirectionalPaddingZ(padding_z);
}
} // namespace Aurora
