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
#else
    uniform vec3 uColor;
#endif

uniform vec3 uViewPos;

void main()
{
    vec3 norm = normalize(vsOut.Normal);
    vec3 viewDir = normalize(uViewPos - vsOut.FragPos);
    float nov = max(dot(norm, viewDir), 0.0);
    float ambient = 0.1;

#ifdef ENABLE_TEXCOORDS
    vec3 objectColor = texture(uTexDiffuse, vsOut.TexCoords).rgb;
#else
    vec3 objectColor = uColor;
#endif

    color = vec4(min(ambient + nov, 1.0) * objectColor, 1.0);
}