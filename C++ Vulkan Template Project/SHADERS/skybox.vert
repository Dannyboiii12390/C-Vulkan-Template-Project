#version 450

layout(push_constant) uniform PushConstantModel {
    mat4 model;  // Only 64 bytes - no texIndex
} pushConstants;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    vec3 redLightPos;
    vec3 eyePos;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBinormal;

layout(location = 0) out vec3 fragTexCoord;

void main() {
    // Remove translation from view matrix for skybox
    mat4 viewNoTranslation = mat4(mat3(ubo.view));
    
    vec4 pos = ubo.proj * viewNoTranslation * pushConstants.model * vec4(inPos, 1.0);
    gl_Position = pos.xyww;  // Ensure skybox is at far plane
    
    fragTexCoord = inPos;  // Use position as cubemap direction
}