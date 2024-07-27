#version 330 core

in VS_Out {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vsOut;

out vec4 color;

void main()
{
    color = vec4(1.0);
}