#version 450

// Input from vertex shader
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inWorldPos;
layout(location = 2) in vec3 inWorldNormal;

// Output
layout(location = 0) out vec4 outColor;

// UBO (binding = 0)
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
} ubo;

// Albedo textures (array of 2)
layout(binding = 1) uniform sampler2D albedoSamplers[2];

// Normal textures (array of 2)
layout(binding = 2) uniform sampler2D normalSamplers[2];

// Skybox cubemap for reflections/refractions
layout(binding = 3) uniform samplerCube skySampler;

// Push constants
layout(push_constant) uniform PushConstants {
    mat4 model;
    int texIndex;
} pushConstants;

void main() {
     vec3 N = normalize(inWorldNormal);
     vec3 V = normalize(ubo.eyePos - inWorldPos);
     float IOR = 1.00 / 1.33; // Index of Refraction (air to water)
     // Calculate the refraction vector
     vec3 R = refract(-V, N, IOR);
     vec3 refractionColor = texture(skySampler, R).rgb;
     outColor = vec4(refractionColor, 1.0);
}
