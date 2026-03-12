#version 430 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gMRA;
layout (location = 4) out vec3 gEmissive;

in VS_Out {
    vec3 FragPos;
    vec3 Normal;
#ifdef ENABLE_TEXCOORDS
    vec2 TexCoords;
#endif
} vsOut;

#ifdef ENABLE_TEXCOORDS
uniform sampler2D uTexBaseColor;
uniform sampler2D uTexDiffuse;
uniform sampler2D uTexSpecular;
uniform sampler2D uTexNormal;
uniform sampler2D uTexMetalness;
uniform sampler2D uTexRoughness;
uniform sampler2D uTexAmbientOcclusion;
uniform sampler2D uTexEmissive;

uniform bool uUsePBR;

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

#ifdef ENABLE_TEXCOORDS
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(uTexNormal, vsOut.TexCoords).xyz * 2.0 - 1.0;

    vec3 q1  = dFdx(vsOut.FragPos);
    vec3 q2  = dFdy(vsOut.FragPos);
    vec2 st1 = dFdx(vsOut.TexCoords);
    vec2 st2 = dFdy(vsOut.TexCoords);

    vec3 n   = normalize(vsOut.Normal);
    vec3 t  = normalize(q1 * st2.t - q2 * st1.t);
    vec3 b  = -normalize(cross(n, t));
    mat3 matTBN = mat3(t, b, n);

    return normalize(matTBN * tangentNormal);
}
#endif

void main()
{
    gPosition = vsOut.FragPos;
#ifdef ENABLE_TEXCOORDS
    vec3 normal = normalize(vsOut.Normal);
    if (uHasNormalMap)
    {
        normal = getNormalFromMap();
    }

    vec3 albedo = vec3(1.0);
    vec3 specular = vec3(0.0);
    float metallic = 0.0;
    float roughness = 1.0;
    float ao = 1.0;
    vec3 emissive = vec3(0.0);

    if (uUsePBR)
    {
        albedo = uBaseColorFactor;
        if (uHasBaseColorMap)
        {
            albedo *= texture(uTexBaseColor, vsOut.TexCoords).rgb;
        }

        metallic = uMetallicFactor;
        roughness = uRoughnessFactor;
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

        ao = uHasAoMap ? texture(uTexAmbientOcclusion, vsOut.TexCoords).r : 1.0;
        emissive = uHasEmissiveMap ? texture(uTexEmissive, vsOut.TexCoords).rgb * uEmissiveFactor : vec3(0.0);

        metallic = clamp(metallic, 0.0, 1.0);
        roughness = clamp(roughness, 0.04, 1.0);
    }
    else
    {
        albedo = texture(uTexDiffuse, vsOut.TexCoords).rgb;
        specular = texture(uTexSpecular, vsOut.TexCoords).rgb;
    }
#else
    vec3 normal = normalize(vsOut.Normal);
    vec3 albedo = uColor;
    float metallic = 0.0;
    float roughness = 1.0;
    float ao = 1.0;
    vec3 emissive = vec3(0.0);
#endif
    gNormal = normal;
#ifdef ENABLE_TEXCOORDS
    // gAlbedo.a stores shading model (1 = PBR, 0 = Phong).
    gAlbedo = vec4(albedo, (uUsePBR ? 1.0 : 0.0));
    gMRA = uUsePBR ? vec3(metallic, roughness, ao) : specular;
#else
    gAlbedo = vec4(albedo, 0.0);
    gMRA = vec3(metallic, roughness, ao);
#endif
    gEmissive = emissive;
}
