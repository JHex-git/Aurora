#version 430 core

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
const int MAX_LIGHTS = 3;

struct Light
{
    vec3 lightPos;
    vec3 lightColor;
    vec2 cullDistance;
    float lightIntensity;
};

layout(std140) uniform LightBlock
{
    Light lights[MAX_LIGHTS];
    int numLights;
} uLightBlock;
uniform samplerCubeArray uTexPointLightShadowMaps;

uniform mat4 uDirectionalLightOrthoVP;
uniform vec3 uDirectionalLightDirection;
uniform vec3 uDirectionalLightColor;
uniform float uDirectionalLightIntensity;
uniform float uDirectionalLightNearFarNorm;
uniform bool uEnableDirectionalLightShadow;
uniform sampler2D uDirectionalLightTexShadowMap;

#define POINT_LIGHT_SELF_SHADOW_BIAS 1e-4
#define DIRECTIONAL_LIGHT_SELF_SHADOW_BIAS 5e-4
#define SELF_SHADOW_FACTOR 0.1

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

// perspective transformation result in zClip:
// zClip = \frac{far + near}{far - near} + \frac{1}{depth} * (\frac{-2\cdot far \cdot near}{far - near})
// So depth = \frac{-2\cdot far\cdot near}{zClip(far - near)-far-near}
float convertPerspectiveClipZToLinear(float z, float zNear, float zFar)
{
    return -2.0 * zNear * zFar / (z * (zFar - zNear) - zFar - zNear);
}

// z01 = zClip / 2 + 0.5
// so depth = \frac{far\cdot near}{far -z01(far-near)}
float convertPerspective01ZToLinear(float z, float zNear, float zFar)
{
    return zFar * zNear / (zFar - z * (zFar - zNear));
}

int getPointLightVisibility(float lightDistance, vec3 lightDir, vec3 normal, int light_index, float zNear, float zFar)
{
    if (dot(lightDir, normal) <= 0) return 0;
    // The maximum axis of dir is the face receives the depth
    // so the distance can be calculated by similarity
    vec4 texCoord = vec4(-lightDir, float(light_index));
    float occluderDistance = convertPerspective01ZToLinear(texture(uTexPointLightShadowMaps, texCoord).r, zNear, zFar);
    occluderDistance /= max(abs(lightDir.x), max(abs(lightDir.y), abs(lightDir.z)));
    return lightDistance < occluderDistance + clamp(SELF_SHADOW_FACTOR * (1 - dot(lightDir, normal)), POINT_LIGHT_SELF_SHADOW_BIAS, SELF_SHADOW_FACTOR) ? 1 : 0;   
}

int getDirectionalLightVisibility(vec3 lightDir, vec3 normal)
{
    if (dot(lightDir, normal) <= 0) return 0;
    vec4 coords = uDirectionalLightOrthoVP * vec4(vsOut.FragPos, 1);
    coords /= coords.w;

    // if not covered by the shadow map, default lit
    if (any(greaterThanEqual(abs(coords.xyz), vec3(1 - 1e-3)))) return 1;

    vec2 ortho_coords = (coords.xy + 1) / 2;
    float occluderDistance = texture(uDirectionalLightTexShadowMap, ortho_coords).r * uDirectionalLightNearFarNorm;
    float lightDistance = (coords.z + 1) / 2 * uDirectionalLightNearFarNorm;
    return lightDistance < occluderDistance + clamp(SELF_SHADOW_FACTOR * (1 - dot(lightDir, normal)), DIRECTIONAL_LIGHT_SELF_SHADOW_BIAS, SELF_SHADOW_FACTOR) ? 1 : 0;
}

vec3 calculateSpecular(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 lightContrib)
{
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    return specularStrength * spec * lightContrib;
}

void PointLighting(vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, inout vec3 lighting)
{
    for (int light_index = 0; light_index < uLightBlock.numLights; ++light_index)
    {
        Light light = uLightBlock.lights[light_index];
        vec3 lightDir = normalize(light.lightPos - vsOut.FragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        float lightDistance = length(light.lightPos - vsOut.FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * lightDistance + 0.032 * lightDistance * lightDistance);
        vec3 lightContrib = light.lightIntensity * light.lightColor * attenuation;
        vec3 diffuse = diff * lightContrib;

        vec3 specular = calculateSpecular(lightDir, normal, viewDir, lightContrib);

        int visibility = getPointLightVisibility(lightDistance, lightDir, normal, light_index, light.cullDistance.x, light.cullDistance.y);

        lighting += (diffuse * diffuseColor + specular * specularColor) * visibility;
    }
}

void DirectionalLighting(vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor, inout vec3 lighting)
{
    vec3 lightDir = -uDirectionalLightDirection;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 lightContrib = uDirectionalLightIntensity * uDirectionalLightColor;
    vec3 diffuse = diff * lightContrib;
    vec3 specular = calculateSpecular(lightDir, normal, viewDir, lightContrib);
    int visibility = getDirectionalLightVisibility(lightDir, normal);

    vec3 result = (diffuse * diffuseColor + specular * specularColor) * visibility;
    lighting += result;
}

void main()
{
    // Diffuse
#ifdef ENABLE_TEXCOORDS
    vec3 normal = getNormalFromMap();
#else
    vec3 normal = normalize(vsOut.Normal);
#endif

#ifdef ENABLE_TEXCOORDS
    vec3 diffuseColor = texture(uTexDiffuse, vsOut.TexCoords).rgb;
    vec3 specularColor = texture(uTexSpecular, vsOut.TexCoords).rgb;
#else
    vec3 diffuseColor = uColor;
    vec3 specularColor = diffuseColor;
#endif

    float ambientStrength = 0.1;
    vec3 lighting = ambientStrength * diffuseColor;
    vec3 viewDir = normalize(uViewPos - vsOut.FragPos);
    if (uEnableDirectionalLightShadow)
    {
        DirectionalLighting(normal, viewDir, diffuseColor, specularColor, lighting);
    }
    PointLighting(normal, viewDir, diffuseColor, specularColor, lighting);
    color = vec4(lighting, 1.0);
}