#version 430 core
out vec4 color;

in vec2 vTexCoord;

uniform sampler2D uGBufferPosition;
uniform sampler2D uGBufferNormal;
uniform sampler2D uGBufferAlbedo;
uniform sampler2D uGBufferSpecular;

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

#define POINT_LIGHT_SELF_SHADOW_BIAS 1e-4
#define DIRECTIONAL_LIGHT_SELF_SHADOW_BIAS 5e-4
#define SELF_SHADOW_FACTOR 0.1

// z01 = zClip / 2 + 0.5
// so depth = \frac{far\cdot near}{far -z01(far-near)}
float convertPerspective01ZToLinear(float z, float zNear, float zFar)
{
    return zFar * zNear / (zFar - z * (zFar - zNear));
}

int getPointLightVisibility(float lightDistance, vec3 lightDir, vec3 normal, int light_index, float zNear, float zFar)
{
    if (dot(lightDir, normal) <= 0) return 0;
    vec4 texCoord = vec4(-lightDir, float(light_index));
    float occluderDistance = convertPerspective01ZToLinear(texture(uTexPointLightShadowMaps, texCoord).r, zNear, zFar);
    occluderDistance /= max(abs(lightDir.x), max(abs(lightDir.y), abs(lightDir.z)));
    return lightDistance < occluderDistance + clamp(SELF_SHADOW_FACTOR * (1 - dot(lightDir, normal)), POINT_LIGHT_SELF_SHADOW_BIAS, SELF_SHADOW_FACTOR) ? 1 : 0;
}

int selectDirectionalCascade(float viewDepth)
{
    int cascade_index = max(uDirectionalCascadeCount - 1, 0);
    for (int i = 0; i < DIRECTIONAL_CASCADE_COUNT; ++i)
    {
        if (i >= uDirectionalCascadeCount) break;
        if (viewDepth <= uDirectionalLightCascadeSplits[i])
        {
            cascade_index = i;
            break;
        }
    }
    return cascade_index;
}

int getDirectionalLightVisibility(vec3 lightDir, vec3 normal, vec3 frag_pos)
{
    if (dot(lightDir, normal) <= 0) return 0;

    float viewDepth = -(uView * vec4(frag_pos, 1.0)).z;
    if (viewDepth <= 0.0) return 1;
    int cascade_index = selectDirectionalCascade(viewDepth);

    vec4 coords = uDirectionalLightOrthoVP[cascade_index] * vec4(frag_pos, 1);
    coords /= coords.w;

    if (any(greaterThanEqual(abs(coords.xyz), vec3(1 - 1e-3)))) return 1;

    vec2 ortho_coords = (coords.xy + 1) / 2;
    float occluderDistance = texture(uDirectionalLightTexShadowMap, vec3(ortho_coords, float(cascade_index))).r * uDirectionalLightNearFarNorm[cascade_index];
    float lightDistance = (coords.z + 1) / 2 * uDirectionalLightNearFarNorm[cascade_index];
    return lightDistance < occluderDistance + clamp(SELF_SHADOW_FACTOR * (1 - dot(lightDir, normal)), DIRECTIONAL_LIGHT_SELF_SHADOW_BIAS, SELF_SHADOW_FACTOR) ? 1 : 0;
}

vec3 calculateSpecular(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 lightContrib)
{
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    return specularStrength * spec * lightContrib;
}

void PointLighting(vec3 frag_pos, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, inout vec3 lighting)
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

        int visibility = getPointLightVisibility(lightDistance, lightDir, normal, light_index, light.cullDistance.x, light.cullDistance.y);

        lighting += (diffuse * diffuseColor + specular * specularColor) * visibility;
    }
}

void DirectionalLighting(vec3 frag_pos, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, inout vec3 lighting)
{
    vec3 lightDir = -uDirectionalLightDirection;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 lightContrib = uDirectionalLightIntensity * uDirectionalLightColor;
    vec3 diffuse = diff * lightContrib;
    vec3 specular = calculateSpecular(lightDir, normal, viewDir, lightContrib);
    int visibility = getDirectionalLightVisibility(lightDir, normal, frag_pos);

    vec3 result = (diffuse * diffuseColor + specular * specularColor) * visibility;
    lighting += result;
}

void main()
{
    vec3 frag_pos = texture(uGBufferPosition, vTexCoord).rgb;
    vec3 normal = texture(uGBufferNormal, vTexCoord).rgb;
    vec3 diffuseColor = texture(uGBufferAlbedo, vTexCoord).rgb;
    vec3 specularColor = texture(uGBufferSpecular, vTexCoord).rgb;

    if (length(normal) < 1e-4)
    {
        color = vec4(0.0);
        return;
    }

    normal = normalize(normal);
    vec3 viewDir = normalize(uViewPos - frag_pos);
    float ambientStrength = 0.1;
    vec3 lighting = ambientStrength * diffuseColor;
    if (uEnableDirectionalLightShadow)
    {
        DirectionalLighting(frag_pos, normal, viewDir, diffuseColor, specularColor, lighting);
    }
    PointLighting(frag_pos, normal, viewDir, diffuseColor, specularColor, lighting);
    color = vec4(lighting, 1.0);
}
