#version 450

// UBO (binding = 0) - specify std140 so GLSL layout matches the C++ UniformBufferObject
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

// Push constants
layout(push_constant) uniform PushConstants {
    mat4 model;
} pushConstants;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec4 fragColor;
layout(location = 1) flat out int outRender; // flat required for integer varyings

// Simple hash function for pseudo-random values based on particle ID
float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}
float mag(vec3 v) {
    return sqrt(dot(v, v));
}

void main() {
    float particleId = float(gl_VertexIndex);

    // Per-particle pseudo-random seeds
    float rnd0 = hash(particleId);
    float rnd1 = hash(particleId + 100.0);
    float rnd2 = hash(particleId + 200.0);
    float rnd3 = hash(particleId + 300.0);

    // Rain-specific parameters
    float baseSpeed = 4.0; // m/s minimum
    float speed = baseSpeed + rnd0 * 8.0;       // 4..12 m/s
    float startHeight = 8.0 + rnd1 * 25.0;     // 8..33 meters above emitter
    float horizontalSpread = (rnd2 - 0.5) * 10.0; // -5..5 offset

    // Lifetime — looped so particles respawn at top.
    float period = 5.0; // seconds for a full spawn -> fall cycle
    float lifetimeOffset = rnd3 * period;
    float t = mod(ubo.time + lifetimeOffset, period);
    float lifeRatio = t / period;

    // Starting position is the provided base mesh position + random offsets
    vec3 pos = inPosition;
    pos.x += horizontalSpread;          // random horizontal placement
    pos.y += startHeight;               // spawn above the emitter
    pos.z += (hash(particleId + 400.0) - 0.5) * 10.0;

    // Gravity-like fall (simple kinematic fall)
    float g = 9.8;
    pos.y = pos.y - speed * t - 0.5 * g * t * t;

    // Wind sway to give some horizontal motion
    float windFreq = 1.0 + rnd0 * 2.5;
    float windStrength = 0.5 + rnd1 * 1.5;
    pos.x += sin(ubo.time * windFreq + rnd2 * 6.283185) * windStrength * (1.0 - lifeRatio);

    // Apply model transform
    vec3 worldPos = (pushConstants.model * vec4(pos, 1.0)).xyz;

    // Transform to clip space
    gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.0);

    // Distance-based sizing (keep streaks longer when closer)
    float viewDepth = length((ubo.view * vec4(worldPos, 1.0)).xyz);
    float sizeScale = clamp(300.0 / max(viewDepth, 0.0001), 0.5, 400.0);

    // Streak length: faster drops produce longer point sprites
    float streakBase = 6.0;
    gl_PointSize = clamp(streakBase * (1.0 + speed * 0.25) * (sizeScale * 0.01), 3.0, 64.0);

    // Color: stronger blue tint for rain, slightly brighter for faster drops.
    float alpha = clamp(0.75 + (speed - baseSpeed) / 8.0 * 0.25, 0.75, 1.0);
    vec3 baseColor = vec3(0.12, 0.40, 1.00);
    float speedTint = mix(0.95, 1.2, (speed - baseSpeed) / 8.0);
    fragColor = vec4(baseColor * speedTint, alpha);

    // Use int 0/1 instead of bool to communicate visibility
    if (worldPos.y < 0.0 || mag(worldPos) >= 99) {
        outRender = 0; // Cull drops below ground
    } else {
        outRender = 1;
    }
}