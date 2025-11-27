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
} ubo;

// Inputs from vertex shader (locations must match)
layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vFragPos;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vNormal;

// Output
layout(location = 0) out vec4 outColor;

// Textures (bindings — adjust descriptor set in host code if different)
layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D normalMap;

// Simple parameters
const float ambientStrength = 0.06;
const float specularStrength = 1.0;
const float shininess = 64.0;

// Calculates ambient+diffuse; returns specular through out parameter
vec3 CalcLightBase(vec3 lightPos, vec3 lightColor, float intensity, vec3 N, vec3 V, vec3 fragPos, out vec3 outSpec)
{
    vec3 L = normalize(lightPos - fragPos);
    float diff = max(dot(N, L), 0.0);

    // Blinn-Phong half vector
    vec3 H = normalize(L + V);
    float specFactor = pow(max(dot(N, H), 0.0), shininess);

    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = diff * lightColor;
    vec3 specular = specularStrength * specFactor * lightColor;

    outSpec = intensity * specular;
    return intensity * (ambient + diffuse);
}

void main()
{
    // IMPORTANT: this shader assumes your albedo textures are created with an sRGB view
    // (e.g. VK_FORMAT_R8G8B8A8_SRGB / VK_FORMAT_B8G8R8A8_SRGB) and your swapchain is sRGB.
    // In that case the sampled albedo is already linear and you MUST NOT manually linearize
    // or gamma-correct in the shader — doing both causes the "double-gamma" dimming.
    //
    // If you do NOT use sRGB image views / swapchain, use the alternative shader that
    // manually linearizes (pow(albedo, vec3(2.2))) and gamma-corrects before output.

    // Sample albedo (assumed linear because image view = SRGB)
    vec4 albedoSample = texture(albedoMap, vUV);
    vec3 albedo = albedoSample.rgb;

    // Sample normal map (assumed in tangent space, encoded in [0,1])
    vec3 nMap = texture(normalMap, vUV).rgb;
    vec3 tangentSpaceNormal = normalize(nMap * 2.0 - 1.0);
    // If your normal map was authored with a flipped green channel, uncomment:
    // tangentSpaceNormal.y = -tangentSpaceNormal.y;

    // Reconstruct TBN from interpolated vertex data (already orthonormalized in vertex shader)
    vec3 T = normalize(vTangent);
    vec3 B = normalize(vBitangent);
    vec3 N = normalize(vNormal);
    mat3 TBN = mat3(T, B, N); // columns: tangent, bitangent, normal

    // Convert normal from tangent space to world space
    vec3 N_world = normalize(TBN * tangentSpaceNormal);

    // View vector
    vec3 V = normalize(ubo.eyePos - vFragPos);

    // Accumulate lighting from sun and moon, separating specular so it is not modulated by albedo
    vec3 baseLighting = vec3(0.0);
    vec3 specularAccum = vec3(0.0);
    vec3 specOut;

    baseLighting += CalcLightBase(ubo.sun_pos, ubo.sun_color, ubo.sun_intensity, N_world, V, vFragPos, specOut);
    specularAccum += specOut;
    baseLighting += CalcLightBase(ubo.moon_pos, ubo.moon_color, ubo.moon_intensity, N_world, V, vFragPos, specOut);
    specularAccum += specOut;

    // Combine: albedo modulates ambient+diffuse, specular is added on top (no manual gamma here)
    vec3 colorLinear = albedo * baseLighting + specularAccum;

    // Clamp in linear space (no gamma correction — swapchain is assumed sRGB)
    vec3 color = clamp(colorLinear, 0.0, 1.0);

    outColor = vec4(color, albedoSample.a);
}