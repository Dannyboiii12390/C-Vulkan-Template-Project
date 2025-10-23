#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    vec3 eyePos;
} ubo;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inWorldNormal;
layout(location = 2) in vec3 inWorldPos;

layout(location = 0) out vec4 outColor;

void main() {
    // Lighting parameters
    vec3 lightColor = vec3(1.0, 1.0, 1.0);        // white light
    vec3 ambientMaterial = vec3(0.0, 1.0, 0.0);   // green ambient
    vec3 diffMaterial = vec3(1.0, 1.0, 1.0);      // white diffuse material

    // Diffuse calculation (using interpolated world-space normal and position)
    vec3 norm = normalize(inWorldNormal);
    vec3 lightDir = normalize(ubo.lightPos - inWorldPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 diffuse = diff * diffMaterial * lightColor;
    vec3 ambient = ambientMaterial * lightColor;
    vec3 finalColor = ambient + diffuse;

    outColor = vec4(finalColor, 1.0);
}