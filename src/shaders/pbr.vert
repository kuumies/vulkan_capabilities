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
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 eyeNormal;
layout(location = 2) out vec3 eyePos;
layout(location = 3) out mat3 tbn;
//layout(location = 4) out mat3 tbnInverse;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec3 t = normalize(vec3(matrices.normal * vec4(inTangent,   0.0)));
    vec3 b = normalize(vec3(matrices.normal * vec4(inBitangent, 0.0)));
    vec3 n = normalize(vec3(matrices.normal * vec4(inNormal,    0.0)));

    gl_Position = matrices.projection *
                  matrices.view *
                  matrices.model * vec4(inPosition, 1.0);
    texCoord  = inTexCoord * 4.0;
    eyeNormal = mat3(matrices.normal) * inNormal;
    eyePos    = vec3(matrices.view * matrices.model * vec4(inPosition, 1.0));
    tbn        = mat3(t, b, n);
//    tbnInverse = transpose(tbn);
}
