#version 330 core
out vec4 FragColor;

in vec2 vTexCoord;

uniform sampler2D uHDRTexture;
uniform float uExposure;

void main()
{
    vec3 hdr_color = texture(uHDRTexture, vTexCoord).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdr_color * uExposure);
    mapped = pow(mapped, vec3(1.0 / 2.2));
    FragColor = vec4(mapped, 1.0);
}
