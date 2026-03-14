#ifndef AURORA_UTILS_GLSL
#define AURORA_UTILS_GLSL

// z01 = zClip / 2 + 0.5
// so depth = \frac{far\cdot near}{far -z01(far-near)}
float convertPerspective01ZToLinear(float z, float zNear, float zFar)
{
    return zFar * zNear / (zFar - z * (zFar - zNear));
}

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

#endif
