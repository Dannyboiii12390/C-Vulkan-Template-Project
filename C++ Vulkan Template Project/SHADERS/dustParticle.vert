#version 450

// UBO must match C++ std140 UniformBufferObject
layout(std140, binding = 0) uniform UniformBufferObject {
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

// Push constants (per-system model)
layout(push_constant) uniform PushConstants {
    mat4 model;
} pushConstants;

layout(location = 0) in vec3 inPosition; // ignored, generated procedurally

layout(location = 0) out vec4 fragColor;

layout(location = 1) flat out int toRender;

// Tweakable constants:
// - Change DUST_SPEED to speed up / slow down global cloud drift
// - CLOUD_RADIUS controls radial extent of the cloud
const float DUST_SPEED = 2.0;        // Set this to control cloud drift speed
const float CLOUD_RADIUS = 80.0;     // Horizontal extent of dust cloud (tweak as needed)
const float CLOUD_HEIGHT = 30.0;     // Y range (0..30) as requested
const float PI = 3.14159265359;

// Simple deterministic hash for per-particle randomness
float hash(float n) { return fract(sin(n) * 43758.5453123); }
vec2 hash2(float n) { return vec2(hash(n), hash(n + 12.345)); }

float mag(vec3 v) {
    return sqrt(dot(v, v));
}

void main()
{
    float id = float(gl_VertexIndex);

    // Per-particle random seeds
    float rndA = hash(id + 1.0);
    float rndB = hash(id + 2.0);
    float rndC = hash(id + 3.0);

    // Uniform distribution over circular area: r = sqrt(rand)*R
    float r = sqrt(rndA) * CLOUD_RADIUS;
    float ang = rndB * 2.0 * PI;
    float y = rndC * CLOUD_HEIGHT; // random Y between 0 and 30

    // Base local position originates near centre of the globe
    vec3 localPos = vec3(cos(ang) * r, y, sin(ang) * r);

    // Global drift direction (random but fixed seed). Change seed numbers for different direction.
    vec3 globalDir = normalize(vec3(hash(123.0) - 0.5, 0.0, hash(456.0) - 0.5));
    // Time-driven drift (ubo.time in seconds)
    vec3 drift = globalDir * (DUST_SPEED * ubo.time);

    // Per-particle turbulence/jitter (small, high-frequency) to fake turbulence
    vec2 phase = hash2(id + 10.0);
    float jitterAmp = 2.0 + (hash(id + 20.0) * 2.5); // per-particle amplitude
    float jitterX = sin(ubo.time * (0.6 + hash(id + 30.0) * 1.8) + phase.x * 6.28318) * (jitterAmp * 0.5);
    float jitterZ = cos(ubo.time * (0.7 + hash(id + 40.0) * 1.6) + phase.y * 6.28318) * (jitterAmp * 0.5);
    float jitterY = sin(ubo.time * (0.4 + hash(id + 50.0) * 1.2) + (phase.x+phase.y) * 6.28318) * 0.6;

    vec3 jitter = vec3(jitterX, jitterY, jitterZ) * 0.3;

    // Final world position (starts near origin and drifts)
    vec3 worldPos = (pushConstants.model * vec4(localPos + drift + jitter, 1.0)).xyz;

    // Clip-space transform
    gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.0);

    // Small point size as requested. Scale by view distance so near/far behave sensibly.
    float viewDepth = length((ubo.view * vec4(worldPos, 1.0)).xyz);
    float baseSize = 3.0; // small base point size
    float size = (baseSize / max(viewDepth, 0.0001));
    gl_PointSize = clamp(size, 1.0, 4.0); // keep points small

    // Dust color: sandy / brownish with modest alpha
    // Slight per-particle brightness variation for natural look
    float brightness = 0.7 + 0.6 * hash(id + 60.0);
    float alpha = 0.10 + 0.18 * (1.0 - (viewDepth / 200.0)); // slightly fade with distance
    alpha = clamp(alpha, 0.05, 0.35);

    vec3 sand = vec3(0.78, 0.66, 0.45) * brightness;
    fragColor = vec4(sand, alpha);

    toRender = mag(worldPos) < 99.0 ? 1 : 0;
}