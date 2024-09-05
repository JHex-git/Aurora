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
uniform sampler2D uTexSpecular;
uniform sampler2D uTexNormal;
#else
uniform vec3 uColor;
#endif

uniform vec3 uViewPos;
uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform float uLightIntensity;

#ifdef POINT_LIGHT
uniform float uZNear;
uniform float uZFar;
uniform samplerCube uTexShadowMap;
#else // DIRECTIONAL_LIGHT
uniform mat4 uOrthoVP;
uniform float uNearFarNorm;
uniform vec3 uLightDirection;
uniform sampler2D uTexShadowMap;
#endif

#ifdef POINT_LIGHT
#define SELF_SHADOW_BIAS 1e-4
#define SELF_SHADOW_FACTOR 0.1
#else // DIRECTIONAL_LIGHT
#define SELF_SHADOW_BIAS 5e-4
#define SELF_SHADOW_FACTOR 0.1
#endif

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

#ifdef POINT_LIGHT
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

int getVisibility(float lightDistance, vec3 lightDir, vec3 normal)
{
    if (dot(lightDir, normal) <= 0) return 0;
    // The maximum axis of dir is the face receives the depth
    // so the distance can be calculated by similarity
    float occluderDistance = convertPerspective01ZToLinear(texture(uTexShadowMap, -lightDir).r, uZNear, uZFar);
    occluderDistance /= max(abs(lightDir.x), max(abs(lightDir.y), abs(lightDir.z)));
    return lightDistance < occluderDistance + clamp(SELF_SHADOW_FACTOR * (1 - dot(lightDir, normal)), SELF_SHADOW_BIAS, SELF_SHADOW_FACTOR) ? 1 : 0;   
}
#else // DIRECTIONAL_LIGHT
int getVisibility(vec3 lightDir, vec3 normal)
{
    if (dot(lightDir, normal) <= 0) return 0;
    vec4 coords = uOrthoVP * vec4(vsOut.FragPos, 1);
    coords /= coords.w;

    // if not covered by the shadow map, default lit
    if (any(greaterThanEqual(abs(coords.xyz), vec3(1 - 1e-3)))) return 1;

    vec2 ortho_coords = (coords.xy + 1) / 2;
    float occluderDistance = texture(uTexShadowMap, ortho_coords).r * uNearFarNorm;
    float lightDistance = (coords.z + 1) / 2 * uNearFarNorm;
    return lightDistance < occluderDistance + clamp(SELF_SHADOW_FACTOR * (1 - dot(lightDir, normal)), SELF_SHADOW_BIAS, SELF_SHADOW_FACTOR) ? 1 : 0;
}
#endif

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * uLightColor;

    // Diffuse
#ifdef ENABLE_TEXCOORDS
    vec3 normal = getNormalFromMap();
#else
    vec3 normal = normalize(vsOut.Normal);
#endif
#ifdef POINT_LIGHT
    vec3 lightDir = normalize(uLightPos - vsOut.FragPos);
#else // DIRECTIONAL_LIGHT
    vec3 lightDir = -uLightDirection;
#endif
    float diff = max(dot(normal, lightDir), 0.0);

#ifdef POINT_LIGHT
    float lightDistance = length(uLightPos - vsOut.FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * lightDistance + 0.032 * lightDistance * lightDistance);
    vec3 lightContrib = uLightIntensity * uLightColor * attenuation;
#else // DIRECTIONAL_LIGHT
    vec3 lightContrib = uLightIntensity * uLightColor;
#endif
    vec3 diffuse = diff * lightContrib;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(uViewPos - vsOut.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightContrib;

#ifdef ENABLE_TEXCOORDS
    vec3 objectColor = texture(uTexDiffuse, vsOut.TexCoords).rgb;
    vec3 specularColor = texture(uTexSpecular, vsOut.TexCoords).rgb;
#else
    vec3 objectColor = uColor;
    vec3 specularColor = objectColor;
#endif

#ifdef POINT_LIGHT
    int visibility = getVisibility(lightDistance, lightDir, normal);
#else // DIRECTIONAL_LIGHT
    int visibility = getVisibility(lightDir, normal);
#endif
    vec3 result = ambient * objectColor + (diffuse * objectColor + specular * specularColor) * visibility;
    color = vec4(result, 1.0);
}