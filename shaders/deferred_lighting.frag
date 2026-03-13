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

#define POINT_LIGHT_SELF_SHADOW_BIAS 1e-4
#define DIRECTIONAL_LIGHT_SELF_SHADOW_BIAS 5e-4
#define SELF_SHADOW_FACTOR 0.1

const int kPointShadowSampleMax = 9;
const int kDirectionalShadowSampleMax = 9;

// z01 = zClip / 2 + 0.5
// so depth = \frac{far\cdot near}{far -z01(far-near)}
float convertPerspective01ZToLinear(float z, float zNear, float zFar)
{
    return zFar * zNear / (zFar - z * (zFar - zNear));
}

float getPointLightVisibility(float lightDistance, vec3 lightDir, vec3 normal, int light_index, float zNear, float zFar)
{
    float nDotL = dot(normal, lightDir);
    if (nDotL <= 0.0) return 0.0;
    if (lightDistance >= zFar) return 1.0;
    float diskRadius = 1.0 / float(textureSize(uTexPointLightShadowMaps, 0).x);

    int kernel = clamp(uPointShadowPcfSamples, 1, kPointShadowSampleMax);
    float half_kernel = 0.5 * float(kernel - 1);
    vec3 dir = -lightDir;
    vec3 up = abs(dir.y) > 0.999 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    vec3 tangent = normalize(cross(up, dir));
    vec3 bitangent = cross(dir, tangent);

    float visibility = 0.0;
    for (int x = 0; x < kernel; ++x)
    {
        for (int y = 0; y < kernel; ++y)
        {
            vec2 offset = (vec2(x, y) - vec2(half_kernel)) * diskRadius;
            vec3 sampleDir = normalize(dir + tangent * offset.x + bitangent * offset.y);
            float denom = dot(normal, sampleDir);
            float nDotSample = max(-denom, 0.0);
            float bias = clamp(SELF_SHADOW_FACTOR * (1.0 - nDotSample), POINT_LIGHT_SELF_SHADOW_BIAS, SELF_SHADOW_FACTOR);
            if (denom < -1e-4)
            {
                float planeDepth = -lightDistance * nDotL / denom;
                bias += max(0.0, lightDistance - planeDepth);
            }
            vec4 texCoord = vec4(sampleDir, float(light_index));
            float occluderDistance = convertPerspective01ZToLinear(texture(uTexPointLightShadowMaps, texCoord).r, zNear, zFar);
            occluderDistance /= max(abs(sampleDir.x), max(abs(sampleDir.y), abs(sampleDir.z)));
            visibility += lightDistance < occluderDistance + bias ? 1.0 : 0.0;
        }
    }
    return visibility / float(kernel * kernel);
}

int selectDirectionalCascade(float viewDepth)
{
    int cascade_index = max(uDirectionalCascadeCount - 1, 0);
    for (int i = 0; i < uDirectionalCascadeCount; ++i)
    {
        if (viewDepth <= uDirectionalLightCascadeSplits[i])
        {
            cascade_index = i;
            break;
        }
    }
    return cascade_index;
}

vec3 getCascadeDebugColor(int cascade_index)
{
    const vec3 colors[4] = vec3[](
        vec3(0.92, 0.16, 0.16),
        vec3(0.16, 0.78, 0.28),
        vec3(0.22, 0.49, 0.86),
        vec3(0.95, 0.78, 0.16)
    );
    return colors[clamp(cascade_index, 0, 3)];
}

float getDirectionalLightVisibility(vec3 lightDir, vec3 normal, vec3 frag_pos)
{
    float nDotL = dot(normal, lightDir);
    if (nDotL <= 0.0) return 0.0;

    float viewDepth = -(uView * vec4(frag_pos, 1.0)).z;
    if (viewDepth <= 0.0) return 1.0;
    int cascade_index = selectDirectionalCascade(viewDepth);

    vec4 coords = uDirectionalLightOrthoVP[cascade_index] * vec4(frag_pos, 1);
    coords /= coords.w;

    if (any(greaterThanEqual(abs(coords.xyz), vec3(1 - 1e-3)))) return 1.0;

    vec2 ortho_coords = (coords.xy + 1) / 2;
    float lightDistance = (coords.z + 1) / 2 * uDirectionalLightNearFarNorm[cascade_index];
    float bias = clamp(SELF_SHADOW_FACTOR * (1.0 - nDotL), DIRECTIONAL_LIGHT_SELF_SHADOW_BIAS, SELF_SHADOW_FACTOR);
    vec2 texelSize = 1.0 / vec2(textureSize(uDirectionalLightTexShadowMap, 0).xy);

    int kernel = clamp(uDirectionalShadowPcfSamples, 1, kDirectionalShadowSampleMax);
    int half_kernel = kernel / 2;
    float visibility = 0.0;
    for (int x = 0; x < kernel; ++x)
    {
        for (int y = 0; y < kernel; ++y)
        {
            vec2 offset = vec2(x - half_kernel, y - half_kernel) * texelSize;
            vec2 uv = ortho_coords + offset;
            if (any(lessThan(uv, vec2(0.0))) || any(greaterThan(uv, vec2(1.0))))
            {
                visibility += 1.0;
                continue;
            }
            float occluderDistance = texture(uDirectionalLightTexShadowMap, vec3(uv, float(cascade_index))).r * uDirectionalLightNearFarNorm[cascade_index];
            visibility += lightDistance < occluderDistance + bias ? 1.0 : 0.0;
        }
    }
    return visibility / float(kernel * kernel);
}

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
}

void PointLighting(vec3 frag_pos, vec3 normal, vec3 viewDir, vec3 albedo, float metallic, float roughness, vec3 F0, inout vec3 lighting)
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

vec3 calculateSpecular(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 lightContrib)
{
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    return specularStrength * spec * lightContrib;
}

void PointLightingPhong(vec3 frag_pos, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, inout vec3 lighting)
{
    for (int light_index = 0; light_index < uLightBlock.numLights; ++light_index)
    {
        Light light = uLightBlock.lights[light_index];
        vec3 lightDir = normalize(light.lightPos - frag_pos);
        float diff = max(dot(normal, lightDir), 0.0);
        float lightDistance = length(light.lightPos - frag_pos);
        float attenuation = 1.0 / (1.0 + 0.09 * lightDistance + 0.032 * lightDistance * lightDistance);
        vec3 lightContrib = light.lightIntensity * light.lightColor * attenuation;
        vec3 diffuse = diff * lightContrib;
        vec3 specular = calculateSpecular(lightDir, normal, viewDir, lightContrib);

        float visibility = getPointLightVisibility(lightDistance, lightDir, normal, light_index, light.cullDistance.x, light.cullDistance.y);

        lighting += (diffuse * diffuseColor + specular * specularColor) * visibility;
    }
}

void DirectionalLightingPhong(vec3 frag_pos, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, inout vec3 lighting)
{
    vec3 lightDir = -uDirectionalLightDirection;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 lightContrib = uDirectionalLightIntensity * uDirectionalLightColor;
    vec3 diffuse = diff * lightContrib;
    vec3 specular = calculateSpecular(lightDir, normal, viewDir, lightContrib);
    float visibility = getDirectionalLightVisibility(lightDir, normal, frag_pos);

    vec3 result = (diffuse * diffuseColor + specular * specularColor) * visibility;
    lighting += result;
}

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
