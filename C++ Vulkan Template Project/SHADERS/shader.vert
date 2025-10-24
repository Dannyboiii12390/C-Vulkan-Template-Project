#version 450

layout(push_constant) uniform PushConstantModel {
    mat4 model;
} pushConstants;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 lightPos;
    vec4 redLightPos;
    vec4 eyePos;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragWorldNormal;
layout(location = 2) out vec3 fragWorldPos;

void main() {
    mat4 model = pushConstants.model;
    vec4 worldPos = model * vec4(inPos, 1.0);
    fragWorldPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    fragWorldNormal = normalize(normalMatrix * inNormal);

    fragColor = vec3(0.0,1.0,0.0);

    gl_Position = ubo.proj * ubo.view * worldPos;
}