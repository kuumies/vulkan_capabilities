/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Shadowmap fragment shader.
 * ---------------------------------------------------------------- */

#version 450

layout(location = 0) out float outDepth;

void main()
{
    outDepth = 1.0f; //gl_FragDepth;
}
