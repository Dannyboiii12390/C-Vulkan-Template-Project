#version 450

// Push constants (matches vertex)
layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

// UBO (binding = 0) - must match application layout
layout(std140, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
    vec3 eyePos;
    vec3 sun_pos;
    vec3 sun_color;
    float sun_intensity;
    vec3 moon_pos;
    vec3 moon_color;
    float moon_intensity;

    float time;
    int inside_globe;
} ubo;

// Inputs from vertex shader (locations must match)
layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vFragPos;


// Output
layout(location = 0) out vec4 outColor;

// Textures (bindings — adjust descriptor set in host code if different)
layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D normalMap;


void main()
{
    // Sample texture only — no lighting
    vec4 tex = texture(albedoMap, vUV) * 2;
    outColor = tex;
}