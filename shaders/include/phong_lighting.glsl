#ifndef AURORA_PHONG_LIGHTING_GLSL
#define AURORA_PHONG_LIGHTING_GLSL

vec3 calculateSpecular(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 lightContrib)
{
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    return specularStrength * spec * lightContrib;
}

void PointLightingPhong(vec3 frag_pos, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, inout vec3 lighting)
{
    for (int light_index = 0; light_index < uLightBlock.numLights; ++light_index)
    {
        Light light = uLightBlock.lights[light_index];
        vec3 lightDir = normalize(light.lightPos - frag_pos);
        float diff = max(dot(normal, lightDir), 0.0);
        float lightDistance = length(light.lightPos - frag_pos);
        float attenuation = 1.0 / (1.0 + 0.09 * lightDistance + 0.032 * lightDistance * lightDistance);
        vec3 lightContrib = light.lightIntensity * light.lightColor * attenuation;
        vec3 diffuse = diff * lightContrib;
        vec3 specular = calculateSpecular(lightDir, normal, viewDir, lightContrib);

        float visibility = getPointLightVisibility(lightDistance, lightDir, normal, light_index, light.cullDistance.x, light.cullDistance.y);

        lighting += (diffuse * diffuseColor + specular * specularColor) * visibility;
    }
}

void DirectionalLightingPhong(vec3 frag_pos, vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, inout vec3 lighting)
{
    vec3 lightDir = -uDirectionalLightDirection;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 lightContrib = uDirectionalLightIntensity * uDirectionalLightColor;
    vec3 diffuse = diff * lightContrib;
    vec3 specular = calculateSpecular(lightDir, normal, viewDir, lightContrib);
    float visibility = getDirectionalLightVisibility(lightDir, normal, frag_pos);

    vec3 result = (diffuse * diffuseColor + specular * specularColor) * visibility;
    lighting += result;
}

#endif
