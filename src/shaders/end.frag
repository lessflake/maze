#version 450 core

// Creates the pixel shifting blue portal effect at the end

in vec3 pos;

out vec4 color;

uniform float time;

// Pseudo-RNG function pulled from some corner of the internet that seems
// to be common, not perfect but works fine (I did not make this) - 
// just to generate random numbers to create a pixel texture effect
float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    // Cut off 16th since wall/floor textures are generated at 16x16
    // so pixel effect matches walls/floors
    vec3 tiered = (0.5 + pos) * 16.0;
    highp ivec3 cut = ivec3(tiered);
    tiered = cut / 16.0;
    // Cyan color
    color.xyz = vec3(0.0, 1.0, 1.0);

    // Randomise green/red components
    color.z -= 0.1*rand(vec2(tiered.yz));
    color.y -= 0.3*rand(vec2(color.z, tiered.x));

    // Pulsate alpha and create a wavy effect upwards
    color.w = 
        0.3 * (1.0 - tiered.z) +
        (1.0 - tiered.z) *
        (1.0 + sin(1.5*(5.0*(1.5-tiered.z) + time/300))) * 0.5;
}
