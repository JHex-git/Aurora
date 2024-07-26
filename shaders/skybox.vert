#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uMVP;

out vec3 TexCoords;

void main()
{
    vec4 position = uMVP * vec4(aPos, 1.0);
    gl_Position = position.xyww;
    TexCoords = aPos;
}