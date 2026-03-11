#pragma once
// std include
#include <algorithm>
#include <memory>
// thirdparty include

// Aurora include
#include "thirdparty/tinyxml2/tinyxml2.h"
#include "Utility/Serializable.h"

namespace Aurora
{

class SceneObject;

class RenderSettings : public Serializable
{
public:
    static constexpr int kDirectionalCascadeMax = 4;
    static constexpr int kDirectionalShadowMapSizeDefault = 1024;
    static constexpr float kDirectionalCascadeSplitLambdaDefault = 0.95f;
    static constexpr float kDirectionalCascadePaddingXYDefault = 2.0f;
    static constexpr float kDirectionalCascadePaddingZDefault = 5.0f;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    int GetDirectionalCascadeCount() const { return m_directional_cascade_count; }
    int GetDirectionalShadowMapSize() const { return m_directional_shadow_map_size; }
    float GetDirectionalSplitLambda() const { return m_directional_split_lambda; }
    float GetDirectionalPaddingXY() const { return m_directional_padding_xy; }
    float GetDirectionalPaddingZ() const { return m_directional_padding_z; }

    void SetDirectionalCascadeCount(int value)
    {
        m_directional_cascade_count = std::clamp(value, 1, kDirectionalCascadeMax);
    }
    void SetDirectionalShadowMapSize(int value)
    {
        m_directional_shadow_map_size = std::max(1, value);
    }
    void SetDirectionalSplitLambda(float value)
    {
        m_directional_split_lambda = std::clamp(value, 0.0f, 1.0f);
    }
    void SetDirectionalPaddingXY(float value)
    {
        m_directional_padding_xy = std::max(0.0f, value);
    }
    void SetDirectionalPaddingZ(float value)
    {
        m_directional_padding_z = std::max(0.0f, value);
    }

private:
    int m_directional_cascade_count = kDirectionalCascadeMax;
    int m_directional_shadow_map_size = kDirectionalShadowMapSizeDefault;
    float m_directional_split_lambda = kDirectionalCascadeSplitLambdaDefault;
    float m_directional_padding_xy = kDirectionalCascadePaddingXYDefault;
    float m_directional_padding_z = kDirectionalCascadePaddingZDefault;
};
} // namespace Aurora
