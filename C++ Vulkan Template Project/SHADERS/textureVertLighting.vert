#version 450

// Push constants (matches fragment)
layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

// UBO (binding = 0)
layout(std140, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    vec3 redLightPos;
    vec3 eyePos;
    float time;
} ubo;

// Vertex inputs (match Engine::Vertex layout)
// location 0: pos, 1: color (unused here), 2: normal, 3: texCoord, 4: tangent, 5: bitangent
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

// Outputs to fragment shader (must match frag locations)
layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;
layout(location = 3) out vec3 vTangent;
layout(location = 4) out vec3 vBitangent;

void main()
{
    // Model-space -> world-space
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    vWorldPos = worldPos.xyz;

    // Standard MVP
    mat4 viewModel = ubo.view * pc.model;
    gl_Position = ubo.proj * viewModel * vec4(inPosition, 1.0);

    // Normal matrix (inverse-transpose of model for correct normal transform)
    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));
    vec3 N = normalize(normalMatrix * inNormal);

    // Transform tangent/bitangent by model (vectors) and orthogonalize to N
    mat3 model3 = mat3(pc.model);
    vec3 T = normalize(model3 * inTangent);
    // Gram-Schmidt orthogonalize T relative to N
    T = normalize(T - N * dot(N, T));
    // Recompute B as cross to ensure right-handed orthonormal frame
    vec3 B = normalize(cross(N, T));

    vNormal = N;
    vTangent = T;
    vBitangent = B;

    vTexCoord = inTexCoord;
}