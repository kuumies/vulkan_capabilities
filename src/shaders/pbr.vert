/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   PBR vertex shader.
 * -------------------------------------------------------------------------- */

#version 450

// -----------------------------------------------------------------------------

layout(binding = 0) uniform Matrices
{
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 normal;
    mat4 light;

} matrices;

// -----------------------------------------------------------------------------

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

// -----------------------------------------------------------------------------

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 worldNormal;
layout(location = 2) out vec3 worldPos;
layout(location = 3) out vec4 lightPos;
layout(location = 4) out mat3 tbn;

// -----------------------------------------------------------------------------

void main()
{
    vec3 t = normalize(vec3(matrices.normal * vec4(inTangent,   0.0)));
    vec3 b = normalize(vec3(matrices.normal * vec4(inBitangent, 0.0)));
    vec3 n = normalize(vec3(matrices.normal * vec4(inNormal,    0.0)));

    // re-orthogonalize T with respect to N
    t = normalize(t - dot(t, n) * n);
    b = cross(n, t);

    gl_Position = matrices.projection *
                  matrices.view *
                  matrices.model * vec4(inPosition, 1.0);
    texCoord  = inTexCoord * 4.0;
    worldNormal = mat3(matrices.normal) * inNormal;
    worldPos    = vec3(matrices.model * vec4(inPosition, 1.0));
    tbn        = mat3(t, b, n);

    const mat4 biasMat = mat4(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.5, 0.5, 0.0, 1.0);

    lightPos = biasMat * matrices.light * matrices.model * vec4(inPosition, 1.0);
}
