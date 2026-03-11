#version 430 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gSpecular;

in VS_Out {
    vec3 FragPos;
    vec3 Normal;
#ifdef ENABLE_TEXCOORDS
    vec2 TexCoords;
#endif
} vsOut;

#ifdef ENABLE_TEXCOORDS
uniform sampler2D uTexDiffuse;
uniform sampler2D uTexSpecular;
uniform sampler2D uTexNormal;
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
    gNormal = getNormalFromMap();
    vec3 albedo = texture(uTexDiffuse, vsOut.TexCoords).rgb;
    vec3 specular = texture(uTexSpecular, vsOut.TexCoords).rgb;
#else
    gNormal = normalize(vsOut.Normal);
    vec3 albedo = uColor;
    vec3 specular = uColor;
#endif
    gAlbedo = vec4(albedo, 1.0);
    gSpecular = specular;
}
