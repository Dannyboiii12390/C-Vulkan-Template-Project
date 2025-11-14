#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 lightPos;
    vec4 redLightPos;
    vec4 eyePos;
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragTexCoord;

void main() {
    fragTexCoord = inPosition;
    
    // Remove translation from view matrix to keep skybox centered
    mat4 viewNoTranslation = mat4(mat3(ubo.view));
    
    vec4 pos = ubo.proj * viewNoTranslation * vec4(inPosition, 1.0);
    
    // Set z = w to ensure skybox is always at far plane
    gl_Position = pos.xyww;
}