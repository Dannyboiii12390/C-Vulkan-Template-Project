#version 450

// UBO (binding = 0) - must match C++ std140 layout
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

layout(location = 0) in vec3 inPosition;
layout(location = 1) flat out int outRender; // flat required for integer varyings
layout(location = 0) out vec4 fragColor;


// Small deterministic hash for per-particle randomness
float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}
vec2 hash2(float n) {
    return vec2(hash(n), hash(n + 12.34));
}
float mag(vec3 v) {
    return sqrt(dot(v, v));
}

void main() {
    float particleId = float(gl_VertexIndex);

    // Per-particle random seeds
    float r0 = hash(particleId * 0.1234);
    float r1 = hash(particleId * 7.321 + 1.0);
    float r2 = hash(particleId * 3.1415 + 2.0);
    float r3 = hash(particleId * 2.718 + 3.0);

    // Snow lifetime (seconds) - staggered so particles loop at different times
    float lifeSpan = mix(6.0, 18.0, r0); // 6..18s
    float localTime = mod(ubo.time + r1 * 100.0, lifeSpan);
    float life = localTime / lifeSpan; // 0..1 (age)

    // Start from mesh-provided position; mesh should distribute particles in a volume
    vec3 pos = inPosition;

    // Vertical fall: accelerate slightly for natural feel, wrap when particle finished its life
    float fallDistance = mix(40.0, 200.0, r2); // how far the particle falls before looping
    float gravity = 0.5 + r3 * 1.5; // per-particle gravity tweak
    // simple quadratic fall profile (t and t^2) for slightly accelerating fall
    float t = life;
    pos.y -= (fallDistance * (t + 0.5 * gravity * t * t));

    // Gentle wind and turbulence:
    // wind direction/time-driven; per-particle phase offsets to decorrelate motion
    vec2 phase = hash2(particleId + 5.0);
    float windStrength = 1.5; // global tuning
    float windNoiseX = sin(ubo.time * 0.1 + phase.x * 6.28318) * 0.75;
    float windNoiseZ = cos(ubo.time * 0.12 + phase.y * 6.28318) * 0.75;
    pos.x += (windNoiseX + hash(particleId + 9.0) * 0.5 - 0.25) * windStrength * (0.5 + 0.8 * t);
    pos.z += (windNoiseZ + hash(particleId + 10.0) * 0.5 - 0.25) * windStrength * (0.5 + 0.8 * t);

    // Small high-frequency flutter to simulate snowflake tumble
    pos.x += sin(ubo.time * (1.0 + r0 * 3.0) + particleId) * 0.15 * (1.0 - t);
    pos.z += cos(ubo.time * (1.1 + r1 * 2.5) + particleId) * 0.15 * (1.0 - t);

    // Apply per-system model transform
    vec3 worldPos = (pushConstants.model * vec4(pos, 1.0)).xyz;

    // Transform to clip space
    gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.0);

    // Size: scaled down to ~20% of previous size
    float viewDist = length((ubo.view * vec4(worldPos, 1.0)).xyz);
    float baseSize = mix(4.0, 16.0, r2);                // per-flake size variation
    float ageFade = smoothstep(0.0, 0.15, life) * (1.0 - smoothstep(0.75, 1.0, life));
    // Multiply final computed size by 0.2 to make particles ~20% of original.
    // Adjust clamp max accordingly (48.0 * 0.2 = 9.6).
    gl_PointSize = clamp((300.0 / max(viewDist, 0.0001)) * baseSize * (0.6 + 0.8 * ageFade) * 0.4, 0.0, 9.6);

    // Color: soft bluish-white with alpha driven by age and a tiny per-particle variation
    float alpha = clamp(0.85 * ageFade * (0.6 + 0.4 * hash(particleId + 11.0)), 0.0, 1.0);
    fragColor = vec4(0.95, 0.97, 1.0, alpha);

        // Use int 0/1 instead of bool to communicate visibility
    if (worldPos.y < 0.0 || mag(worldPos) >= 99) {
        outRender = 0; // Cull drops below ground
    } else {
        outRender = 1;
    }
}