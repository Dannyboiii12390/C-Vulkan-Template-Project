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
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBinormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragWorldNormal;
layout(location = 2) out vec3 fragWorldPos;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) out vec3 fragLightPos_tangent;
layout(location = 5) out vec3 fragViewPos_tangent;
layout(location = 6) out vec3 fragPos_tangent;

void main() {
    mat4 model = pushConstants.model;
    vec4 worldPos = model * vec4(inPos, 1.0);
    fragWorldPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    fragWorldNormal = normalize(normalMatrix * inNormal);

    fragColor = vec3(0.0,1.0,0.0);
    fragTexCoord = inTexCoord;
    gl_Position = ubo.proj * ubo.view * worldPos;

    Mat4 ModelMatrix_TInv= transpose(inverse(pushConstants.model);
    vec3 T = normalize(mat3(ModelMatrix_TInv) * inTangent);
    vec3 B = normalize(mat3(ModelMatrix_TInv) * inBinormal);
    vec3 N = normalize(mat3(ModelMatrix_TInv) * inNormal);
    mat3 TBN = transpose(mat3(T, B, N)); // Use transpose to invert

    // Get world-space light and view positions
    vec3 lightPos_world = ubo.lightPos;
    vec3 viewPos_world = ubo.viewPos;
    vec3 fragPos_world = (pushConstants.model * vec4(inPosition, 1.0)).xyz;

    // Transform light and view POSITIONS to tangent space
    fragLightPos_tangent = TBN * lightPos_world;
    fragViewPos_tangent = TBN * viewPos_world;
    fragPos_tangent = TBN * fragPos_world;
}