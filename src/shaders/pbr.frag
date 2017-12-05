/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   PBR fragment shader.
 * ---------------------------------------------------------------- */

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform Light
{
    vec4 dir;
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

void main()
{
    vec3 n = normalize(eyeNormal);
    vec3 l = normalize(light.dir.rgb);
    vec3 v = normalize(-eyePos);

    outColor = texture(baseColorMap, texCoord);
    outColor = vec4(outColor.rgb * dot(n, l), 1.0);
}
