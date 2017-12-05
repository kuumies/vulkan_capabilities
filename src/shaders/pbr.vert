/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   PBR vertex shader.
 * ---------------------------------------------------------------- */

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform Matrices
{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 normal;

} matrices;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 eyeNormal;
layout(location = 2) out vec3 eyePos;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = matrices.projection *
                  matrices.view *
                  matrices.model * vec4(inPosition, 1.0);
    texCoord  = inTexCoord;
    eyeNormal = mat3(matrices.normal) * inNormal;
    eyePos    = vec3(matrices.view * matrices.model * vec4(inPosition, 1.0));
}
