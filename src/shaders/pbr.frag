/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   PBR fragment shader.
 * ---------------------------------------------------------------- */

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform Light
{
    vec4 eyeDir;
    vec4 intensity;

} light;

layout(binding = 2) uniform sampler2D ambientOcclusionMap;
layout(binding = 3) uniform sampler2D baseColorMap;
layout(binding = 4) uniform sampler2D heightMap;
layout(binding = 5) uniform sampler2D metallicMap;
layout(binding = 6) uniform sampler2D normalMap;
layout(binding = 7) uniform sampler2D roughnessMap;

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 eyeNormal;
layout(location = 2) in vec3 eyePos;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159;

float brdfNormalDistributionGGX(vec3 n, vec3 h, float a)
{

    float a2     = a * a;
    float nDotH  = max(dot(n, h), 0.0);
    float nDotH2 = nDotH * nDotH;
    float q      = (nDotH2 * (a2 -1.0) + 1.0);
    return a2 / (PI * q * q);
}

float brdfGeometryGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float brdfGeometry(vec3 n, vec3 v, vec3 l, float k)
{
    float nDotV = max(dot(n, v), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    float ggx1 = brdfGeometryGGX(nDotV, k);
    float ggx2 = brdfGeometryGGX(nDotL, k);

    return ggx1 * ggx2;
}

vec3 brdfFresnel(float cosTheta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    // Sample maps
    float metallic  = texture(metallicMap, texCoord).r;
    float roughness = texture(roughnessMap, texCoord).r;
    float ao        = texture(ambientOcclusionMap, texCoord).r;
    vec3 albedo     = texture(baseColorMap, texCoord).rgb;
    //albedo = pow(albedo, vec3(2.2));
    vec3 normal     = texture(normalMap, texCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    // Calculate vectors.
    //vec3 n = normalize(eyeNormal);
    vec3 n = normalize(normal);
    vec3 l = normalize(-light.eyeDir.xyz);
    vec3 v = normalize(-eyePos);
    vec3 h = normalize(l + v);

    float nDotL = max(dot(n, l), 0.0);
    vec3 f0 = mix(vec3(0.04), albedo, metallic);

    float ndf = brdfNormalDistributionGGX(n, h, roughness*roughness);
    float g   = brdfGeometry(n, v, l, roughness);
    vec3 f    = brdfFresnel(nDotL, f0);

    vec3 kS = f;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 nominator    = ndf * g * f;
    float denominator = 4.0 * max(dot(n, v), 0.0) * nDotL;
    vec3 specular     = nominator / max(denominator, 0.001);

    vec3 radiance = light.intensity.rgb;
    radiance = (kD * albedo / PI + specular) * radiance * nDotL;

    vec3 ambient = albedo * ao;
    outColor.rgb = ambient + radiance;
    outColor.a = 1.0;

    // reinhard tone mapping
    outColor = outColor / (outColor + vec4(1.0));

    // gamma correction
//    outColor = pow(outColor, vec4(1.0 / 2.2));
//    outColor.a = 1.0;
}
