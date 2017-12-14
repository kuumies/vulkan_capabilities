/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   IBL prefilter vertex shader.
 * ---------------------------------------------------------------- */

#version 450

layout(binding = 0) uniform Uniforms
{
    mat4 view;
    mat4 projection;
    float roughness;
} matrices;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 texCoord;

void main()
{
    gl_Position = matrices.projection * matrices.view * vec4(inPosition, 1.0);
    texCoord = inPosition;
}
