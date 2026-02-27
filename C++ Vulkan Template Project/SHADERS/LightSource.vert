#version 450

// Push constants (matches fragment)
layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

// UBO (binding = 0)
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

// Vertex inputs (Engine::Vertex layout)
// location 0: pos, 1: color (unused), 2: normal, 3: texCoord, 4: tangent, 5: bitangent
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

// Outputs to fragment shader (must match frag locations)
layout(location = 0) out vec2 vUV;
layout(location = 1) out vec3 vFragPos;

void main()
{
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    vFragPos = worldPos.xyz;
    vUV = inTexCoord;

    gl_Position = ubo.proj * ubo.view * worldPos;
}