/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   IBL prefilter fragment shader.
 * -------------------------------------------------------------------------- */

#version 450

// -----------------------------------------------------------------------------

layout(location = 0) in vec2 texCoord;

// -----------------------------------------------------------------------------

layout(location = 0) out vec2 outColor;

// -----------------------------------------------------------------------------

const float PI = 3.14159265359;

// -----------------------------------------------------------------------------

float geometrySchlickGGX(float nDotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0;

    float nom   = nDotV;
    float denom = nDotV * (1.0 - k) + k;

    return nom / denom;
}

// -----------------------------------------------------------------------------

float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// -----------------------------------------------------------------------------

vec2 hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}

// -----------------------------------------------------------------------------

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// -----------------------------------------------------------------------------

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

// -----------------------------------------------------------------------------

vec2 integrateBRDF(float nDotV, float roughness)
{
    vec3 v;
    v.x = sqrt(1.0 - nDotV*nDotV);
    v.y = 0.0;
    v.z = nDotV;

    float a = 0.0;
    float b = 0.0;

    vec3 n = vec3(0.0, 0.0, 1.0);

    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 xi = hammersley(i, SAMPLE_COUNT);
        vec3 h  = importanceSampleGGX(xi, n, roughness);
        vec3 l  = normalize(2.0 * dot(v, h) * h - v);

        float nDotL = max(l.z, 0.0);
        float nDotH = max(h.z, 0.0);
        float vDotH = max(dot(v, h), 0.0);

        if(nDotL > 0.0)
        {
            float g = geometrySmith(n, v, l, roughness);
            float gVis = (g * vDotH) / (nDotH * nDotV);
            float fc = pow(1.0 - vDotH, 5.0);

            a += (1.0 - fc) * gVis;
            b += fc * gVis;
        }
    }
    a /= float(SAMPLE_COUNT);
    b /= float(SAMPLE_COUNT);
    return vec2(a, b);
}

// ----------------------------------------------------------------------------
void main()
{
    vec2 integratedBRDF = integrateBRDF(texCoord.x, texCoord.y);
    outColor = integratedBRDF;
}
