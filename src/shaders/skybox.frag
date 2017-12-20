/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Skybox fragment shader.
 * -------------------------------------------------------------------------- */

#version 450

// -----------------------------------------------------------------------------

layout(binding = 1) uniform samplerCube skyboxMap;

// -----------------------------------------------------------------------------

layout(location = 0) in vec3 texCoord;

// -----------------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

// -----------------------------------------------------------------------------

void main()
{
    outColor = texture(skyboxMap, texCoord);
    // reinhard tone mapping
    outColor = outColor / (outColor + vec4(1.0));
}
