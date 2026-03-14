#ifndef AURORA_SHADOW_COMMON_GLSL
#define AURORA_SHADOW_COMMON_GLSL

#ifndef POINT_LIGHT_SELF_SHADOW_BIAS
#define POINT_LIGHT_SELF_SHADOW_BIAS 1e-4
#endif

#ifndef DIRECTIONAL_LIGHT_SELF_SHADOW_BIAS
#define DIRECTIONAL_LIGHT_SELF_SHADOW_BIAS 5e-4
#endif

#ifndef SELF_SHADOW_FACTOR
#define SELF_SHADOW_FACTOR 0.1
#endif

const int kPointShadowSampleMax = 9;
const int kDirectionalShadowSampleMax = 9;

#include "utils.glsl"
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

#endif
