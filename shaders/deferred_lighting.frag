#version 430 core
out vec4 color;

in vec2 vTexCoord;

uniform sampler2D uGBufferPosition;
uniform sampler2D uGBufferNormal;
uniform sampler2D uGBufferAlbedo;
uniform sampler2D uGBufferMRA;
uniform sampler2D uGBufferEmissive;

uniform vec3 uViewPos;
uniform mat4 uView;

const int MAX_LIGHTS = 4;

struct Light
{
    vec3 lightPos;
    vec3 lightColor;
    vec2 cullDistance;
    float lightIntensity;
};

layout(std140) uniform LightBlock
{
    Light lights[MAX_LIGHTS];
    int numLights;
} uLightBlock;

uniform samplerCubeArray uTexPointLightShadowMaps;

#ifndef DIRECTIONAL_CASCADE_COUNT
#define DIRECTIONAL_CASCADE_COUNT 4
#endif

uniform mat4 uDirectionalLightOrthoVP[DIRECTIONAL_CASCADE_COUNT];
uniform float uDirectionalLightCascadeSplits[DIRECTIONAL_CASCADE_COUNT];
uniform int uDirectionalCascadeCount;
uniform vec3 uDirectionalLightDirection;
uniform vec3 uDirectionalLightColor;
uniform float uDirectionalLightIntensity;
uniform float uDirectionalLightNearFarNorm[DIRECTIONAL_CASCADE_COUNT];
uniform bool uEnableDirectionalLightShadow;
uniform sampler2DArray uDirectionalLightTexShadowMap;
uniform int uDirectionalShadowPcfSamples;
uniform int uPointShadowPcfSamples;
uniform bool uDebugShowCascades;

#include "shadow_common.glsl"
#include "pbr_lighting.glsl"
#include "phong_lighting.glsl"
void main()
{
    vec3 frag_pos = texture(uGBufferPosition, vTexCoord).rgb;
    vec3 normal = texture(uGBufferNormal, vTexCoord).rgb;
    vec4 albedoPacked = texture(uGBufferAlbedo, vTexCoord);
    vec3 albedo = albedoPacked.rgb;
    vec3 mra = texture(uGBufferMRA, vTexCoord).rgb;
    vec3 emissive = texture(uGBufferEmissive, vTexCoord).rgb;

    if (length(normal) < 1e-4)
    {
        color = vec4(0.0);
        return;
    }

    normal = normalize(normal);
    vec3 viewDir = normalize(uViewPos - frag_pos);
    if (uDebugShowCascades && uEnableDirectionalLightShadow)
    {
        float viewDepth = -(uView * vec4(frag_pos, 1.0)).z;
        int cascade_index = selectDirectionalCascade(viewDepth);
        color = vec4(getCascadeDebugColor(cascade_index), 1.0);
        return;
    }
    // gAlbedo.a encodes shading model: 0 = Phong, 1 = PBR.
    if (albedoPacked.a < 0.5)
    {
        vec3 specularColor = mra;
        float ambientStrength = 0.1;
        vec3 lighting = ambientStrength * albedo;
        if (uEnableDirectionalLightShadow)
        {
            DirectionalLightingPhong(frag_pos, normal, viewDir, albedo, specularColor, lighting);
        }
        PointLightingPhong(frag_pos, normal, viewDir, albedo, specularColor, lighting);
        color = vec4(lighting, 1.0);
    }
    else
    {
        // PBR shading path.
        float metallic = clamp(mra.r, 0.0, 1.0);
        float roughness = clamp(mra.g, 0.04, 1.0);
        float ao = clamp(mra.b, 0.0, 1.0);
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 lighting = vec3(0.0);
        if (uEnableDirectionalLightShadow)
        {
            DirectionalLighting(frag_pos, normal, viewDir, albedo, metallic, roughness, F0, lighting);
        }
        PointLighting(frag_pos, normal, viewDir, albedo, metallic, roughness, F0, lighting);
        vec3 ambient = vec3(0.03) * albedo * ao;
        color = vec4(ambient + lighting + emissive, 1.0);
    }
}
