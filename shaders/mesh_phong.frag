#version 430 core

in VS_Out {
    vec3 FragPos;
    vec3 Normal;
#ifdef ENABLE_TEXCOORDS
    vec2 TexCoords;
#endif
} vsOut;

out vec4 color;

#ifdef ENABLE_TEXCOORDS
// Switch between Phong and PBR without changing shader programs.
uniform bool uUsePBR;

uniform sampler2D uTexDiffuse;
uniform sampler2D uTexSpecular;
uniform sampler2D uTexNormal;
uniform sampler2D uTexBaseColor;
uniform sampler2D uTexMetalness;
uniform sampler2D uTexRoughness;
uniform sampler2D uTexAmbientOcclusion;
uniform sampler2D uTexEmissive;

uniform bool uHasBaseColorMap;
uniform bool uHasNormalMap;
uniform bool uHasMetalnessMap;
uniform bool uHasRoughnessMap;
uniform bool uHasAoMap;
uniform bool uHasEmissiveMap;

uniform vec3 uBaseColorFactor;
uniform float uMetallicFactor;
uniform float uRoughnessFactor;
uniform vec3 uEmissiveFactor;
#else
uniform vec3 uColor;
#endif

uniform vec3 uViewPos;
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

uniform mat4 uView;
uniform bool uDebugShowCascades;

#include "shadow_common.glsl"
#include "pbr_lighting.glsl"
#include "phong_lighting.glsl"
#ifdef ENABLE_TEXCOORDS
#include "utils.glsl"
#endif
// perspective transformation result in zClip:
// zClip = \frac{far + near}{far - near} + \frac{1}{depth} * (\frac{-2\cdot far \cdot near}{far - near})
// So depth = \frac{-2\cdot far\cdot near}{zClip(far - near)-far-near}
float convertPerspectiveClipZToLinear(float z, float zNear, float zFar)
{
    return -2.0 * zNear * zFar / (z * (zFar - zNear) - zFar - zNear);
}

void main()
{
    if (uDebugShowCascades && uEnableDirectionalLightShadow)
    {
        float viewDepth = -(uView * vec4(vsOut.FragPos, 1.0)).z;
        int cascade_index = selectDirectionalCascade(viewDepth);
        color = vec4(getCascadeDebugColor(cascade_index), 1.0);
        return;
    }
#ifdef ENABLE_TEXCOORDS
    vec3 normal = normalize(vsOut.Normal);
    if (uUsePBR)
    {
        if (uHasNormalMap)
        {
            normal = getNormalFromMap();
        }
    }
    else
    {
        normal = getNormalFromMap();
    }
#else
    vec3 normal = normalize(vsOut.Normal);
#endif
    vec3 viewDir = normalize(uViewPos - vsOut.FragPos);

#ifdef ENABLE_TEXCOORDS
    if (uUsePBR)
    {
        // PBR shading path.
        vec3 albedo = uBaseColorFactor;
        if (uHasBaseColorMap)
        {
            albedo *= texture(uTexBaseColor, vsOut.TexCoords).rgb;
        }

        float metallic = uMetallicFactor;
        float roughness = uRoughnessFactor;
        if (uHasMetalnessMap)
        {
            vec3 mrSample = texture(uTexMetalness, vsOut.TexCoords).rgb;
            metallic *= mrSample.b;
            if (!uHasRoughnessMap)
            {
                roughness = uRoughnessFactor * mrSample.g;
            }
        }
        if (uHasRoughnessMap)
        {
            roughness = uRoughnessFactor * texture(uTexRoughness, vsOut.TexCoords).r;
        }

        float ao = uHasAoMap ? texture(uTexAmbientOcclusion, vsOut.TexCoords).r : 1.0;
        vec3 emissive = uHasEmissiveMap ? texture(uTexEmissive, vsOut.TexCoords).rgb * uEmissiveFactor : vec3(0.0);

        metallic = clamp(metallic, 0.0, 1.0);
        roughness = clamp(roughness, 0.04, 1.0);

        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 lighting = vec3(0.0);

        if (uEnableDirectionalLightShadow)
        {
            DirectionalLighting(vsOut.FragPos, normal, viewDir, albedo, metallic, roughness, F0, lighting);
        }
        PointLighting(vsOut.FragPos, normal, viewDir, albedo, metallic, roughness, F0, lighting);

        vec3 ambient = vec3(0.03) * albedo * ao;
        vec3 result = ambient + lighting + emissive;
        color = vec4(result, 1.0);
    }
    else
    {
        // Phong shading path.
        vec3 diffuseColor = texture(uTexDiffuse, vsOut.TexCoords).rgb;
        vec3 specularColor = texture(uTexSpecular, vsOut.TexCoords).rgb;
        float ambientStrength = 0.1;
        vec3 lighting = ambientStrength * diffuseColor;
        if (uEnableDirectionalLightShadow)
        {
            DirectionalLightingPhong(vsOut.FragPos, normal, viewDir, diffuseColor, specularColor, lighting);
        }
        PointLightingPhong(vsOut.FragPos, normal, viewDir, diffuseColor, specularColor, lighting);
        color = vec4(lighting, 1.0);
    }
#else
    vec3 diffuseColor = uColor;
    vec3 specularColor = diffuseColor;
    float ambientStrength = 0.1;
    vec3 lighting = ambientStrength * diffuseColor;
    if (uEnableDirectionalLightShadow)
    {
        DirectionalLightingPhong(vsOut.FragPos, normal, viewDir, diffuseColor, specularColor, lighting);
    }
    PointLightingPhong(vsOut.FragPos, normal, viewDir, diffuseColor, specularColor, lighting);
    color = vec4(lighting, 1.0);
#endif
}
