#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
} pushConstants;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    vec3 eyePos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.proj * ubo.view * pushConstants.model * vec4(inPosition, 1.0);
    //fragColor = inColor;

    // Transform position and normal to world space
    vec3 worldPos = (pushConstants.model * vec4(inPosition, 1.0)).xyz;
    vec3 worldNormal = mat3(transpose(inverse(pushConstants.model))) * inNormal;

    // Define light and material properties
    vec3 lightColor = vec3(1.0, 1.0, 1.0); // Light color
    vec3 ambientMaterial = vec3(0.0, 0.1, 0.0); // Ambient light component

    // Diffuse calculation
    vec3 norm = normalize(worldNormal);
    vec3 lightDir = normalize(ubo.lightPos - worldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Combine and pass to fragment shader
    vec3 diffMaterial=vec3(1.0);
    fragColor = ambientMaterial* lightColor;
    fragColor += diffMaterial* lightColor* diffuse;
}