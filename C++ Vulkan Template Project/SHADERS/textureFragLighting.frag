#version 450

layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vFragPos;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vNormal;

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

layout(set = 0, binding = 1) uniform sampler2D albedoMap;
layout(set = 0, binding = 2) uniform sampler2D normalMap;

// Simple material parameters
const float shininess = 32.0;
const vec3 ambientFactor = vec3(0.08);

void main()
{
    // Sample albedo
    vec3 albedo = texture(albedoMap, vUV).rgb;

    // Sample normal map (tangent space) and reconstruct
    vec3 nSample = texture(normalMap, vUV).rgb;
    vec3 nTangent = normalize(nSample * 2.0 - 1.0);

    // Build TBN matrix (tangent, bitangent, normal) -> world space
    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    vec3 N = normalize(TBN * nTangent);

    // Light and view directions (use sun_pos and eyePos from UBO)
    vec3 L = normalize(ubo.sun_pos - vFragPos);
    vec3 V = normalize(ubo.eyePos - vFragPos);
    vec3 H = normalize(L + V);

    // Ambient
    vec3 ambient = ambientFactor * albedo;

    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * albedo;

    // Specular (Blinn-Phong)
    float spec = 0.0;
    if (diff > 0.0) {
        spec = pow(max(dot(N, H), 0.0), shininess);
    }
    vec3 specular = spec * vec3(1.0); // white specular; customize if needed

    vec3 color = ambient + diffuse + specular;
    outColor = vec4(color, 1.0);
}