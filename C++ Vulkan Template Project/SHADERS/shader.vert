#version 450

// Input attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBinormal;

// Output to fragment shader
layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outWorldPos;
layout(location = 2) out vec3 outWorldNormal;

// Uniform Buffer Object
layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 viewPos;
    vec3 lightPos;
    vec3 redLightPos;
} ubo;

// Push constants
layout(push_constant) uniform PushConstants {
    mat4 model;
    int texIndex;
} pushConstants;

void main() {
    // Transform position to world space
    vec4 worldPos = pushConstants.model * vec4(inPosition, 1.0);
    outWorldPos = worldPos.xyz;
    
    // Transform normal to world space (use transpose of inverse for non-uniform scaling)
    outWorldNormal = mat3(transpose(inverse(pushConstants.model))) * inNormal;
    
    // Pass through texture coordinates
    outTexCoord = inTexCoord;
    
    // Transform to clip space
    gl_Position = ubo.proj * ubo.view * worldPos;
}