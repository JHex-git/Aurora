#version 330 core
layout (location = 0) in vec3 aPos;
#ifdef ENABLE_NORMALS
    #define TEXCOORDS_LOCATION 2
    layout (location = 1) in vec3 aNormal;
#else
    #define TEXCOORDS_LOCATION 1
#endif
#ifdef ENABLE_TEXCOORDS
    layout (location = TEXCOORDS_LOCATION) in vec2 aColorTexCoords;
#endif

out VS_Out {
    vec3 FragPos;
#ifdef ENABLE_NORMALS
    vec3 Normal;
#endif
#ifdef ENABLE_TEXCOORDS
    vec2 TexCoords;
#endif
} vsOut;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    vec4 positionWS = uModel * vec4(aPos, 1.0);
    gl_Position = uProjection * uView * positionWS;
    vsOut.FragPos = positionWS.xyz / positionWS.w;
#ifdef ENABLE_NORMALS
    vsOut.Normal = mat3(transpose(inverse(uModel))) * aNormal; // No matter this is normalized, fragment uses this should normalize it as it is interpolated
#endif
#ifdef ENABLE_TEXCOORDS
    vsOut.TexCoords = aColorTexCoords;
#endif
}