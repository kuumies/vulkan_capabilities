/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Skybox fragment shader.
 * ---------------------------------------------------------------- */

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform samplerCube skyboxMap;

layout(location = 0) in vec3 texCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(skyboxMap, texCoord);
}
