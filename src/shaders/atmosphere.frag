/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The fragment shader of atmosphere shader.

   See: http://codeflow.org/entries/2011/apr/13/advanced-webgl-part-2-sky-rendering/
 * -------------------------------------------------------------------------- */
 
#version 450

// -----------------------------------------------------------------------------

layout(binding = 0) uniform Params
{
    vec4 viewport;
    mat4 invProj;
    mat4 invViewRot;
    vec4 lightDir;
    vec4 kr;
    float rayleighBrightness;
    float mieBrightness;
    float spotBrightness;
    float scatterStrength;
    float rayleighStrength;
    float mieStrength;
    float rayleighCollectionPower;
    float mieCollectionPower;
    float mieDistribution;

} data;

// -----------------------------------------------------------------------------

layout(location = 0) in vec2 uv;

// -----------------------------------------------------------------------------

layout(location = 0) out vec4 colorOut;

// -----------------------------------------------------------------------------
// Normal in the world space as seen from this fragment
vec3 getWorldNormal()
{
    vec2 fragCoord = gl_FragCoord.xy / data.viewport.xy;
    fragCoord = (fragCoord - 0.5) * 2.0;
    vec4 deviceNormal = vec4(fragCoord, 0.0, 1.0);
    vec3 eyeNormal = normalize(vec3(data.invProj * deviceNormal));
    vec3 worldNormal = normalize(mat3(data.invViewRot) * eyeNormal);
    return worldNormal;
}

// -----------------------------------------------------------------------------
// Distance to the sphere of 1 radius outer sheel from the position and direction
float atmosphericDepth(vec3 position, vec3 dir)
{
    float a = dot(dir, dir);
    float b = 2.0 * dot(dir, position);
    float c = dot(position, position) - 1.0;
    float det = b * b - 4.0 * a * c;
    float detSqrt = sqrt(det);
    float q = (-b - detSqrt) / 2.0;
    float t1 = c / q;
    return t1;
}

// -----------------------------------------------------------------------------
// Atmosphere scarttering phase function
// see: http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter16.html
float phase(float alpha, float g)
{
    float a = 3.0 * (1.0 - g * g);
    float b = 2.0 * (2.0 + g * g);
    float c = 1.0 + alpha * alpha;
    float d = pow(1.0 + g * g - 2.0 * g * alpha, 1.5);
    return (a / b) * (c / d);
}

// -----------------------------------------------------------------------------
// Atmospheric extinction is the reduction in brightness of stellar objects as their photons pass through our atmosphere.
// The effects of extinction depend on transparency, elevation of the observer, and the zenith angle, the angle from the
// zenith to one’s line of sight. Therefore, looking vertically, the zenith angle is 00, and is 900 at the horizon.
// http://www.asterism.org/tutorials/tut28-1.htm

// dot(a, b) == 0 => 90 degree angle between vectors
// dot(a, b) > 0 => less than 90 degree
// dot(a, b) < 0 => more than 90 degree
// dot(a, a) => square of a's length

float horizonExtinction(vec3 position, vec3 dir, float radius)
{
    float u = dot(dir, -position);
    if(u<0.0)
    {
        return 1.0;
    }
    vec3 near = position + u * dir;
    if(length(near) < radius)
    {
        return 0.0;
    }
    else
    {
        vec3 v2 = normalize(near) * radius - position;
        float diff = acos(dot(normalize(v2), dir));
        return smoothstep(0.0, 1.0, pow(diff * 2.0, 3.0));
    }
}

// -----------------------------------------------------------------------------

vec3 absorb(float dist, vec3 color, float factor)
{
    return color - color * pow(data.kr.xyz, vec3(factor / dist));
}

// -----------------------------------------------------------------------------

void main(void)
{
    vec3 lightDir = normalize(data.lightDir.xyz);
    lightDir.x = -lightDir.x;
    lightDir.y = -lightDir.y;

    const float surfaceHeight = 0.99;
    const float intensity = 2;
    const int stepCount = 32;

    // Eye position on the planet of radius of 1.0f
    vec3 eyePosition = vec3(0.0, surfaceHeight, 0.0);

    // The look-at direction
    vec3 eyedir = getWorldNormal();

    // Distance from the eye position into planet outershell
    float eyeDepth = atmosphericDepth(eyePosition, eyedir);

    // Sample interval
    float stepLength = eyeDepth / float(stepCount);

    vec3 rayleighCollected = vec3(0.0, 0.0, 0.0);
    vec3 mieCollected = vec3(0.0, 0.0, 0.0);


    for(int i = 0; i < stepCount; i++)
    {
        // Sample distance along the view ray
        float sampleDistance = stepLength * float(i);

        // Sample position in plane
        vec3 position = eyePosition + eyedir * sampleDistance;

        // Sample distance along the ligth ray
        float sampleDepth = atmosphericDepth(position, lightDir);

        float extinction = horizonExtinction(position, lightDir, surfaceHeight - 0.35);

        // absorb light for the sample ray from the sun to the sample position
        vec3 influx = absorb(sampleDepth, vec3(intensity), data.scatterStrength) * extinction;

        // Nitrogen reflection
        rayleighCollected += absorb(sampleDistance, data.kr.xyz * influx, data.rayleighStrength);
        mieCollected += absorb(sampleDistance, influx, data.mieStrength);
    }

    float alpha = dot(eyedir, lightDir);
    float rayleighFactor = phase(alpha, -0.01) * data.rayleighBrightness;
    float mieFactor = phase(alpha, data.mieDistribution) * data.mieBrightness;
    float spot = smoothstep(0.0, 15.0, phase(alpha, 0.9995)) * data.spotBrightness;

    float eyeExtinction = horizonExtinction(eyePosition, eyedir, surfaceHeight - 0.15);
    rayleighCollected = (rayleighCollected * eyeExtinction*pow(eyeDepth, data.rayleighCollectionPower)) / float(stepCount);
    mieCollected = (mieCollected * eyeExtinction*pow(eyeDepth, data.mieCollectionPower)) / float(stepCount);

    vec3 color = vec3(spot * mieCollected + mieFactor * mieCollected + rayleighFactor * rayleighCollected);

    colorOut = vec4(color, 1.0);
}
