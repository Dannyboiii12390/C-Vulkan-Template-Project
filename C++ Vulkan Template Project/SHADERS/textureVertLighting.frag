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

// Inputs from vertex shader
layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;

// Texture samplers
layout(binding = 1) uniform sampler2D albedoTex;
layout(binding = 2) uniform sampler2D normalTex;

// Output
layout(location = 0) out vec4 outColor;

void main()
{
    // Sample albedo
    vec3 albedo = texture(albedoTex, vTexCoord).rgb;

    // Sample normal map and transform from [0,1] to [-1,1]
    vec3 normalSample = texture(normalTex, vTexCoord).rgb;
    vec3 tangentNormal = normalize(normalSample * 2.0 - 1.0);

    // Construct TBN matrix
    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));
    vec3 worldNormal = normalize(TBN * tangentNormal);

    // Lighting calculations (example: simple diffuse)
    vec3 lightDir = normalize(ubo.sun_pos - vWorldPos);
    float diff = max(dot(worldNormal, lightDir), 0.0);

    // Simple ambient + diffuse
    vec3 ambient = 0.1 * albedo;
    vec3 diffuse = diff * albedo;

    outColor = vec4(ambient + diffuse, 1.0);
}