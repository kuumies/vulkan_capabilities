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

float brdfNormalDistributionGGX(vec3 n, vec3 h, float a)
{
    const float PI = 3.14159;

    float a2 = a * a;
    float nDotH = max(dot(n, h), 0.0);
    float nDotH2 = nDotH * nDotH;
    float q = (nDotH2 * (a2 -1.0) + 1.0);
    return a2 / (PI * q * q);
}

void main()
{
    vec3 n = normalize(eyeNormal);
    vec3 l = normalize(-light.eyeDir.xyz);
    vec3 v = normalize(-eyePos);
    vec3 h = normalize(l + v);

    float r = texture(roughnessMap, texCoord).r;
    //r = 0.3;
    r = brdfNormalDistributionGGX(n, h, r*r);
    outColor = vec4(r, r, r, 1.0);

    // reinhard tone mapping
    //outColor = outColor / (outColor + vec4(1.0));
    // gamma correction
    //outColor = pow(outColor, vec4(1.0 / 2.2));
    //outColor.a = 1.0;
}
