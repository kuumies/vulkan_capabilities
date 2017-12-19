/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Shadowmap depth fragment shader.
 * ---------------------------------------------------------------- */

#version 450

layout(binding = 0) uniform sampler2D shadowMap;

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

void main()
{
    float d = texture(shadowMap, texCoord).r;
    outColor.rgb = vec3(d, d, d);
    outColor.a = 1.0;
}
