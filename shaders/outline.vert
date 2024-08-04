#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uOutlineThickness;
uniform int uScreenHeight;
uniform int uScreenWidth;

void main()
{
    vec4 positionWS = uModel * vec4(aPos, 1.0);
    vec4 positionCS = uProjection * uView * positionWS;
    vec3 normalWS = mat3(transpose(inverse(uModel))) * aNormal;
    vec3 normalCS = (uProjection * uView * vec4(normalWS, 0.0)).xyz;

    // Dilate in CS to ensure the outline is always same thickness.
    // Divide screen parameters to normalize the thickness.
    // Multiply by 2 to make sure uOutlineThickness is exactly the pixel width.
    positionCS.xy += normalize(normalCS.xy) / ivec2(uScreenWidth, uScreenHeight) * uOutlineThickness * positionCS.w * 2;
    gl_Position = positionCS;
}