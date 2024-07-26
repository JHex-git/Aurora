#version 330 core

in vec3 TexCoords;

uniform samplerCube uCubeSampler;

out vec4 color;

void main()
{
    color = texture(uCubeSampler, normalize(TexCoords));
}