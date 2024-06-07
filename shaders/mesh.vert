#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aColorTexCoords;

out VS_Out {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vsOut;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    vec4 positionWS = uModel * vec4(aPos, 1.0);
    gl_Position = uProjection * uView * positionWS;
    vsOut.FragPos = positionWS.xyz / positionWS.w;
    vsOut.Normal = mat3(transpose(inverse(uModel))) * aNormal; // No matter this is normalized, fragment uses this should normalize it as it is interpolated
    vsOut.TexCoords = aColorTexCoords;
}