#version 450

// Per-frame UBO (view/proj needed to position the globe)
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
    int inside_globe;
} ubo;

// Vertex inputs (Engine::Vertex)
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// Push constant for per-object model matrix
layout(push_constant) uniform PushConstants {
    mat4 model;
    int texIndex; // keep if used by other shaders
} pc;

// Outputs to fragment shader
layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vWorldNormal;
layout(location = 2) out vec2 vUV;

void main()
{
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    vWorldPos = worldPos.xyz;

    // Normal matrix to handle non-uniform scale
    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));
    vWorldNormal = normalize(normalMatrix * inNormal);

    vUV = inTexCoord;

    // Standard transform to clip-space
    gl_Position = ubo.proj * ubo.view * worldPos;
}