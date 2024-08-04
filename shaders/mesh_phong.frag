#version 330 core

in VS_Out {
    vec3 FragPos;
    vec3 Normal;
#ifdef ENABLE_TEXCOORDS
    vec2 TexCoords;
#endif
} vsOut;

out vec4 color;

#ifdef ENABLE_TEXCOORDS
uniform sampler2D uTexDiffuse;
uniform sampler2D uTexSpecular;
uniform sampler2D uTexNormal;
#else
uniform vec3 uColor;
#endif

uniform vec3 uViewPos;
uniform vec3 uLightPos;
uniform vec3 uLightColor;

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

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * uLightColor;

    // Diffuse
#ifdef ENABLE_TEXCOORDS
    vec3 norm = getNormalFromMap();
#else
    vec3 norm = normalize(vsOut.Normal);
#endif
    vec3 lightDir = normalize(uLightPos - vsOut.FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(uViewPos - vsOut.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * uLightColor;

#ifdef ENABLE_TEXCOORDS
    vec3 objectColor = texture(uTexDiffuse, vsOut.TexCoords).rgb;
    vec3 specularColor = texture(uTexSpecular, vsOut.TexCoords).rgb;
#else
    vec3 objectColor = uColor;
    vec3 specularColor = vec3(1.0);
#endif
    vec3 result = (ambient + diffuse) * objectColor + specular * specularColor;
    color = vec4(result, 1.0);
}