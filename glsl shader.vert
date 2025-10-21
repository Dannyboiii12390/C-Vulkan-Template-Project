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
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec3 fragWorldNormal;

void main() {
    // Transform position to clip space
    gl_Position = ubo.proj * ubo.view * pushConstants.model * vec4(inPosition, 1.0);

    // Pass world position and normal to fragment shader
    fragWorldPos = (pushConstants.model * vec4(inPosition, 1.0)).xyz;
    fragWorldNormal = mat3(transpose(inverse(pushConstants.model))) * inNormal;
    
    // Pass base material color to fragment shader
    fragColor = vec3(1.0, 1.0, 1.0);  // White material or could be any base color
}