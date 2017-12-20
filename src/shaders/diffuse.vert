/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Diffuse vertex shader.
 * -------------------------------------------------------------------------- */

#version 450

layout(binding = 0) uniform Matrices
{
    mat4 model;
    mat4 view;
    mat4 projection;

} matrices;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec2 uv;

void main()
{
    gl_Position = matrices.projection *
                  matrices.view *
                  matrices.model * vec4(inPosition, 1.0);
    uv = inTexCoord;
}
