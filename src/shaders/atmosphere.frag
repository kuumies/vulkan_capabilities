/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The fragment shader of atmosphere shader.

   See: http://codeflow.org/entries/2011/apr/13/advanced-webgl-part-2-sky-rendering/
 * ---------------------------------------------------------------- */
 
#version 450

/* ---------------------------------------------------------------- */

layout(binding = 0) uniform Params
{
    vec4 viewport;
    mat4 inv_proj;
    mat4 inv_view_rot;
    vec4 lightdir;
    vec4 Kr;
    float rayleigh_brightness;
    float mie_brightness;
    float spot_brightness;
    float scatter_strength;
    float rayleigh_strength;
    float mie_strength;
    float rayleigh_collection_power;
    float mie_collection_power;
    float mie_distribution;

} data;

/* ---------------------------------------------------------------- */

layout(location = 0) in vec2 uv;

/* ---------------------------------------------------------------- */

layout(location = 0) out vec4 colorOut;

/* ---------------------------------------------------------------- */
// Normal in the world space as seen from this fragment
vec3 get_world_normal()
{
    vec2 frag_coord = gl_FragCoord.xy/data.viewport.xy;
    frag_coord = (frag_coord-0.5)*2.0;
    vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
    vec3 eye_normal = normalize(vec3(data.inv_proj * device_normal));
    vec3 world_normal = normalize(mat3(data.inv_view_rot)*eye_normal);
    return world_normal;
}

/* ---------------------------------------------------------------- */
// Distance to the sphere of 1 radius outer sheel from the position and direction
float atmospheric_depth(vec3 position, vec3 dir){
    float a = dot(dir, dir);
    float b = 2.0*dot(dir, position);
    float c = dot(position, position)-1.0;
    float det = b*b-4.0*a*c;
    float detSqrt = sqrt(det);
    float q = (-b - detSqrt)/2.0;
    float t1 = c/q;
    return t1;
}

/* ---------------------------------------------------------------- */
// Atmosphere scarttering phase function
// see: http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter16.html
float phase(float alpha, float g){
    float a = 3.0*(1.0-g*g);
    float b = 2.0*(2.0+g*g);
    float c = 1.0+alpha*alpha;
    float d = pow(1.0+g*g-2.0*g*alpha, 1.5);
    return (a/b)*(c/d);
}

/* ---------------------------------------------------------------- */
//Atmospheric extinction is the reduction in brightness of stellar objects as their photons pass through our atmosphere.
// The effects of extinction depend on transparency, elevation of the observer, and the zenith angle, the angle from the
// zenith to one’s line of sight. Therefore, looking vertically, the zenith angle is 00, and is 900 at the horizon.
// http://www.asterism.org/tutorials/tut28-1.htm

// dot(a, b) == 0 => 90 degree angle between vectors
// dot(a, b) > 0 => less than 90 degree
// dot(a, b) < 0 => more than 90 degree
// dot(a, a) => square of a's length

float horizon_extinction(vec3 position, vec3 dir, float radius){
    float u = dot(dir, -position);
    if(u<0.0){
        return 1.0;
    }
    vec3 near = position + u*dir;
    if(length(near) < radius){
        return 0.0;
    }
    else{
        vec3 v2 = normalize(near)*radius - position;
        float diff = acos(dot(normalize(v2), dir));
        return smoothstep(0.0, 1.0, pow(diff*2.0, 3.0));
    }
}

/* ---------------------------------------------------------------- */

vec3 absorb(float dist, vec3 color, float factor){
    return color-color*pow(data.Kr.xyz, vec3(factor/dist));
}

/* ---------------------------------------------------------------- */

void main(void)
{
    vec3 lightDir = normalize(data.lightdir.xyz);
    lightDir.x = -lightDir.x;
    lightDir.y = -lightDir.y;
    //lightDir.z = -lightDir.z;
//    colorOut.rgb = vec3(20.0, 10.0, 5.0) * 0.1;
//    colorOut.a = 1.0;
//    return;

    const float surface_height = 0.99;
    const float intensity = 2;
    const int step_count = 32;

    // Eye position on the planet of radius of 1.0f
    vec3 eye_position = vec3(0.0, surface_height, 0.0);

    // The look-at direction
    vec3 eyedir = get_world_normal();
    //colorOut = vec4(eyedir, 1.0);
    //return;

    // Distance from the eye position into planet outershell
    float eye_depth = atmospheric_depth(eye_position, eyedir);

    // Sample interval
    float step_length = eye_depth / float(step_count);

    vec3 rayleigh_collected = vec3(0.0, 0.0, 0.0);
    vec3 mie_collected = vec3(0.0, 0.0, 0.0);


    for(int i = 0; i < step_count; i++)
    {
        // Sample distance along the view ray
        float sample_distance = step_length * float(i);

        // Sample position in plane
        vec3 position = eye_position + eyedir * sample_distance;

        // Sample distance along the ligth ray
        float sample_depth = atmospheric_depth(position, lightDir);

        float extinction = horizon_extinction(position, lightDir, surface_height - 0.35);

        // absorb light for the sample ray from the sun to the sample position
        vec3 influx = absorb(sample_depth, vec3(intensity), data.scatter_strength) * extinction;

        // Nitrogen reflection
        rayleigh_collected += absorb(sample_distance, data.Kr.xyz*influx, data.rayleigh_strength);
        mie_collected += absorb(sample_distance, influx, data.mie_strength);
    }

    float alpha = dot(eyedir, lightDir);
    float rayleigh_factor = phase(alpha, -0.01)*data.rayleigh_brightness;
    float mie_factor = phase(alpha, data.mie_distribution)*data.mie_brightness;
    float spot = smoothstep(0.0, 15.0, phase(alpha, 0.9995))*data.spot_brightness;

    float eye_extinction = horizon_extinction(eye_position, eyedir, surface_height-0.15);
    rayleigh_collected = (rayleigh_collected*eye_extinction*pow(eye_depth, data.rayleigh_collection_power))/float(step_count);
    mie_collected = (mie_collected*eye_extinction*pow(eye_depth, data.mie_collection_power))/float(step_count);

    vec3 color = vec3(spot * mie_collected + mie_factor * mie_collected + rayleigh_factor * rayleigh_collected);

    colorOut = vec4(color, 1.0);
}
