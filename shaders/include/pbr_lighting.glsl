#ifndef AURORA_PBR_LIGHTING_GLSL
#define AURORA_PBR_LIGHTING_GLSL

#include "shadow_common.glsl"

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / max(denom, 1e-4);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float denom = NdotV * (1.0 - k) + k;
    return NdotV / max(denom, 1e-4);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}void PointLighting(vec3 frag_pos, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0, inout vec3 lighting)
{
    for (int light_index = 0; light_index < uLightBlock.numLights; ++light_index)
    {
        Light light = uLightBlock.lights[light_index];
        vec3 lightDir = normalize(light.lightPos - frag_pos);
        float NdotL = max(dot(normal, lightDir), 0.0);
        float lightDistance = length(light.lightPos - frag_pos);
        float attenuation = 1.0 / (1.0 + 0.09 * lightDistance + 0.032 * lightDistance * lightDistance);
        vec3 lightContrib = light.lightIntensity * light.lightColor * attenuation;
        float visibility = getPointLightVisibility(lightDistance, lightDir, normal, light_index, light.cullDistance.x, light.cullDistance.y);

        if (NdotL > 0.0)
        {
            vec3 L = lightDir;
            vec3 H = normalize(viewDir + L);
            vec3 radiance = lightContrib;

            float NDF = DistributionGGX(normal, H, roughness);
            float G = GeometrySmith(normal, viewDir, L, roughness);
            vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);

            vec3 numerator = NDF * G * F;
            float denom = 4.0 * max(dot(normal, viewDir), 0.0) * NdotL + 1e-4;
            vec3 specular = numerator / denom;

            vec3 kS = F;
            vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
            vec3 diffuse = kD * albedo / PI;

            lighting += (diffuse + specular) * radiance * NdotL * visibility;
        }
    }
}

void DirectionalLighting(vec3 frag_pos, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0, inout vec3 lighting)
{
    vec3 lightDir = -uDirectionalLightDirection;

    float visibility = getDirectionalLightVisibility(lightDir, normal, frag_pos);
    float NdotL = max(dot(normal, lightDir), 0.0);
    if (NdotL > 0.0)
    {
        vec3 L = normalize(lightDir);
        vec3 H = normalize(viewDir + L);
        vec3 radiance = uDirectionalLightIntensity * uDirectionalLightColor;

        float NDF = DistributionGGX(normal, H, roughness);
        float G = GeometrySmith(normal, viewDir, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denom = 4.0 * max(dot(normal, viewDir), 0.0) * NdotL + 1e-4;
        vec3 specular = numerator / denom;

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
        vec3 diffuse = kD * albedo / PI;

        lighting += (diffuse + specular) * radiance * NdotL * visibility;
    }
}

#endif
