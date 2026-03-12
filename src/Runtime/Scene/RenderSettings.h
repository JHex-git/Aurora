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
    enum class RenderPath
    {
        Forward,
        Deferred
    };

    static constexpr int kDirectionalCascadeMax = 4;
    static constexpr int kDirectionalShadowMapSizeDefault = 1024;
    static constexpr float kDirectionalCascadeSplitLambdaDefault = 0.95f;
    static constexpr float kDirectionalCascadePaddingXYDefault = 2.0f;
    static constexpr float kDirectionalCascadePaddingZDefault = 5.0f;
    static constexpr int kDirectionalShadowPcfSamplesDefault = 3;
    static constexpr int kDirectionalShadowPcfSamplesMax = 9;
    static constexpr int kPointShadowPcfSamplesDefault = 3;
    static constexpr int kPointShadowPcfSamplesMax = 9;

    void Serialize(tinyxml2::XMLElement *node) override;
    void Deserialize(const tinyxml2::XMLElement *node, std::shared_ptr<SceneObject> owner) override;

    static const char* RenderPathToString(RenderPath path);
    static RenderPath RenderPathFromString(const char* value);

    RenderPath GetRenderPath() const { return m_render_path; }
    int GetDirectionalCascadeCount() const { return m_directional_cascade_count; }
    int GetDirectionalShadowMapSize() const { return m_directional_shadow_map_size; }
    float GetDirectionalSplitLambda() const { return m_directional_split_lambda; }
    float GetDirectionalPaddingXY() const { return m_directional_padding_xy; }
    float GetDirectionalPaddingZ() const { return m_directional_padding_z; }
    int GetDirectionalShadowPcfSamples() const { return m_directional_shadow_pcf_samples; }
    int GetPointShadowPcfSamples() const { return m_point_shadow_pcf_samples; }

    void SetRenderPath(RenderPath value) { m_render_path = value; }
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
    void SetDirectionalShadowPcfSamples(int value)
    {
        m_directional_shadow_pcf_samples = ClampOdd(value, 1, kDirectionalShadowPcfSamplesMax);
    }
    void SetPointShadowPcfSamples(int value)
    {
        m_point_shadow_pcf_samples = ClampOdd(value, 1, kPointShadowPcfSamplesMax);
    }

private:
    RenderPath m_render_path = RenderPath::Forward;
    int m_directional_cascade_count = kDirectionalCascadeMax;
    int m_directional_shadow_map_size = kDirectionalShadowMapSizeDefault;
    float m_directional_split_lambda = kDirectionalCascadeSplitLambdaDefault;
    float m_directional_padding_xy = kDirectionalCascadePaddingXYDefault;
    float m_directional_padding_z = kDirectionalCascadePaddingZDefault;
    int m_directional_shadow_pcf_samples = kDirectionalShadowPcfSamplesDefault;
    int m_point_shadow_pcf_samples = kPointShadowPcfSamplesDefault;

    static int ClampOdd(int value, int min_value, int max_value)
    {
        int clamped = std::clamp(value, min_value, max_value);
        if ((clamped % 2) == 0)
        {
            clamped = (clamped + 1 <= max_value) ? clamped + 1 : clamped - 1;
        }
        return std::clamp(clamped, min_value, max_value);
    }
};
} // namespace Aurora
