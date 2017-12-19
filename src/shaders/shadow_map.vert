/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Shadowmap vertex shader.
 * ---------------------------------------------------------------- */

#version 450

layout(binding = 0) uniform Matrices
{
    mat4 model;
    mat4 light;

} matrices;

layout(location = 0) in vec3 inPosition;

void main()
{
    gl_Position = matrices.light * matrices.model * vec4(inPosition, 1.0);
}
