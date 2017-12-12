/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The vertex shader of atmosphere shader.
 * ---------------------------------------------------------------- */

#version 450
#extension GL_ARB_separate_shader_objects : enable

/* ---------------------------------------------------------------- */

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

layout(location = 0) out vec2 uv;

/* ---------------------------------------------------------------- */

void main(void)
{
    uv = texCoord;
    gl_Position = vec4(position, 1.0);
}
