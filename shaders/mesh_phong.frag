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

#define POINT_LIGHT_SELF_SHADOW_BIAS 1e-4
#define DIRECTIONAL_LIGHT_SELF_SHADOW_BIAS 5e-4
#define SELF_SHADOW_FACTOR 0.1

const int kPointShadowSampleMax = 9;
const int kDirectionalShadowSampleMax = 9;

#ifdef ENABLE_TEXCOORDS
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(uTexNormal, vsOut.TexCoords).xyz * 2.0 - 1.0;

    vec3 q1  = dFdx(vsOut.FragPos);
    vec3 q2  = dFdy(vsOut.FragPos);
    vec2 st1 = dFdx(vsOut.TexCoords);
    vec2 st2 = dFdy(vsOut.TexCoords);

    vec3 n   = normalize(vsOut.Normal);
    vec3 t  = normalize(q1*st2.t - q2*st1.t);
    vec3 b  = -normalize(cross(n, t));
    mat3 matTBN = mat3(t, b, n);

    return normalize(matTBN * tangentNormal);
}
#endif

// perspective transformation result in zClip:
// zClip = \frac{far + near}{far - near} + \frac{1}{depth} * (\frac{-2\cdot far \cdot near}{far - near})
// So depth = \frac{-2\cdot far\cdot near}{zClip(far - near)-far-near}
float convertPerspectiveClipZToLinear(float z, float zNear, float zFar)
{
    return -2.0 * zNear * zFar / (z * (zFar - zNear) - zFar - zNear);
}

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

float getDirectionalLightVisibility(vec3 lightDir, vec3 normal)
{
    float nDotL = dot(normal, lightDir);
    if (nDotL <= 0.0) return 0.0;

    float viewDepth = -(uView * vec4(vsOut.FragPos, 1.0)).z;
    if (viewDepth <= 0.0) return 1.0;
    int cascade_index = selectDirectionalCascade(viewDepth);

    vec4 coords = uDirectionalLightOrthoVP[cascade_index] * vec4(vsOut.FragPos, 1);
    coords /= coords.w;

    // if not covered by the shadow map, default lit
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

vec3 calculateSpecular(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 lightContrib)
{
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    return specularStrength * spec * lightContrib;
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

void PointLighting(vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, inout vec3 lighting)
{
    for (int light_index = 0; light_index < uLightBlock.numLights; ++light_index)
    {
        Light light = uLightBlock.lights[light_index];
        vec3 lightDir = normalize(light.lightPos - vsOut.FragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        float lightDistance = length(light.lightPos - vsOut.FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * lightDistance + 0.032 * lightDistance * lightDistance);
        vec3 lightContrib = light.lightIntensity * light.lightColor * attenuation;
        vec3 diffuse = diff * lightContrib;

        vec3 specular = calculateSpecular(lightDir, normal, viewDir, lightContrib);

        float visibility = getPointLightVisibility(lightDistance, lightDir, normal, light_index, light.cullDistance.x, light.cullDistance.y);

        lighting += (diffuse * diffuseColor + specular * specularColor) * visibility;
    }
}

void DirectionalLighting(vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, inout vec3 lighting)
{
    vec3 lightDir = -uDirectionalLightDirection;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 lightContrib = uDirectionalLightIntensity * uDirectionalLightColor;
    vec3 diffuse = diff * lightContrib;
    vec3 specular = calculateSpecular(lightDir, normal, viewDir, lightContrib);
    float visibility = getDirectionalLightVisibility(lightDir, normal);

    vec3 result = (diffuse * diffuseColor + specular * specularColor) * visibility;
    lighting += result;
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
        vec3 Lo = vec3(0.0);

        for (int light_index = 0; light_index < uLightBlock.numLights; ++light_index)
        {
            Light light = uLightBlock.lights[light_index];
            vec3 L = normalize(light.lightPos - vsOut.FragPos);
            vec3 H = normalize(viewDir + L);
            float distance = length(light.lightPos - vsOut.FragPos);
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
            vec3 radiance = light.lightIntensity * light.lightColor * attenuation;
            float NdotL = max(dot(normal, L), 0.0);
            if (NdotL > 0.0)
            {
                float NDF = DistributionGGX(normal, H, roughness);
                float G = GeometrySmith(normal, viewDir, L, roughness);
                vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);

                vec3 numerator = NDF * G * F;
                float denom = 4.0 * max(dot(normal, viewDir), 0.0) * NdotL + 1e-4;
                vec3 specular = numerator / denom;

                vec3 kS = F;
                vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
                vec3 diffuse = kD * albedo / PI;

                float visibility = getPointLightVisibility(distance, L, normal, light_index, light.cullDistance.x, light.cullDistance.y);
                Lo += (diffuse + specular) * radiance * NdotL * visibility;
            }
        }

        if (uEnableDirectionalLightShadow)
        {
            vec3 L = normalize(-uDirectionalLightDirection);
            vec3 H = normalize(viewDir + L);
            float NdotL = max(dot(normal, L), 0.0);
            if (NdotL > 0.0)
            {
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

                float visibility = getDirectionalLightVisibility(L, normal);
                Lo += (diffuse + specular) * radiance * NdotL * visibility;
            }
        }

        vec3 ambient = vec3(0.03) * albedo * ao;
        vec3 result = ambient + Lo + emissive;
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
            DirectionalLighting(normal, viewDir, diffuseColor, specularColor, lighting);
        }
        PointLighting(normal, viewDir, diffuseColor, specularColor, lighting);
        color = vec4(lighting, 1.0);
    }
#else
    vec3 diffuseColor = uColor;
    vec3 specularColor = diffuseColor;
    float ambientStrength = 0.1;
    vec3 lighting = ambientStrength * diffuseColor;
    if (uEnableDirectionalLightShadow)
    {
        DirectionalLighting(normal, viewDir, diffuseColor, specularColor, lighting);
    }
    PointLighting(normal, viewDir, diffuseColor, specularColor, lighting);
    color = vec4(lighting, 1.0);
#endif
}
