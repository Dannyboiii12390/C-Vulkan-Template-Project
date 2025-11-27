#version 450

// Push constants (matches vertex)
layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

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

// Texture bindings (match descriptor set layout in C++)
layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D normalMap;
layout(binding = 3) uniform samplerCube skyboxMap; // kept for compatibility

// Inputs from vertex shader (locations must match vertex outputs)
layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;
// per-vertex lighting term computed in vertex shader
layout(location = 5) in vec3 vLighting;

// Output
layout(location = 0) out vec4 outColor;

void main()
{
    // Sample albedo
    vec4 albedoSample = texture(albedoMap, vTexCoord);
    vec3 albedo = albedoSample.rgb;
    float alpha = albedoSample.a;

    // Alpha cutoff for simple transparency handling
    if (alpha < 0.05) {
        discard;
    }

    // Sample normal map and convert to [-1,1]
    vec3 nSample = texture(normalMap, vTexCoord).rgb;
    nSample = nSample * 2.0 - 1.0;

    // Reconstruct TBN (tangent, bitangent, normal are provided in world-space)
    vec3 T = normalize(vTangent);
    vec3 B = normalize(vBitangent);
    vec3 N = normalize(vNormal);
    mat3 TBN = mat3(T, B, N); // columns: tangent, bitangent, normal

    // Transform normal-map normal into world space (bumped normal)
    vec3 Nw = normalize(TBN * nSample);

    // Apply vertex-provided lighting to albedo (linear lighting)
    vec3 color = albedo * vLighting;

    // Slight modulation between geometric normal and bumped normal
    color *= mix(0.98, 1.06, clamp(dot(N, Nw) * 0.5 + 0.5, 0.0, 1.0));

    // NOTE: remove manual gamma conversion here.
    // If your swapchain uses an sRGB format (preferred), the GPU will
    // perform the linear->sRGB conversion automatically when writing the color.
    // color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, alpha);
}