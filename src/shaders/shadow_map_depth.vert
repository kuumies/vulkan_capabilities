/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Shadowmap depth vertex shader.
 * ---------------------------------------------------------------- */

#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 texCoord;

void main()
{
    gl_Position = vec4(inPosition, 1.0);
    texCoord = inTexCoord;
}
