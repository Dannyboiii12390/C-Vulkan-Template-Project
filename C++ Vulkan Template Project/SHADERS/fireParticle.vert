#version 450

// Keep UBO layout identical to your project's UBO so offsets match.
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

// Push constants: model matrix
layout(push_constant) uniform PushConstants {
    mat4 model;
} pushConstants;

// Per-vertex base particle position (seed position)
layout(location = 0) in vec3 inPosition;

// Outputs to fragment shader
layout(location = 0) out float vLifeRatio;
layout(location = 1) out float vSpark;      // 1.0 => ember/spark
layout(location = 2) out vec3 vColor;

float hash(float n) { return fract(sin(n) * 43758.5453123); }
float hash2(vec2 v) { return fract(sin(dot(v, vec2(12.9898,78.233))) * 43758.5453123); }

void main() {
    // Per-particle deterministic randomness from gl_VertexIndex
    float id = float(gl_VertexIndex);
    float r1 = hash(id);
    float r2 = hash(id + 37.0);
    float r3 = hash(id + 73.0);

    // Lifetime behavior
    float lifeSpan = 2.2 + r2 * 1.8;           // 2.2..4.0s
    float offset = r3 * lifeSpan;
    float t = mod(ubo.time + offset, lifeSpan);
    float lifeRatio = clamp(t / lifeSpan, 0.0, 1.0);
    vLifeRatio = lifeRatio;

    // Determine if particle is a bright ember/spark (rare)
    vSpark = (r1 > 0.96) ? 1.0 : 0.0;

    // Motion parameters
    // multiply initial upward speed by sqrt(2) to double peak height:
    const float HEIGHT_SCALE = 1.41421356237; // sqrt(2)
    float upwardSpeed = (2.5 + r1 * 2.0) * HEIGHT_SCALE;         // scaled initial upward speed
    float gravity = 0.45;                       // current gravity
    float swirl = 1.0 + r2 * 2.5;               // horizontal spread multiplier
    float swirlSpeed = 1.5 + r3 * 2.0;

    // Base animated local position
    vec3 pos = inPosition;

    // Vertical motion: rise then slow by gravity
    float y = upwardSpeed * t - 0.5 * gravity * t * t;
    pos.y += y;

    // Horizontal swirl / turbulence (time-varying)
    float ang = (r2 * 6.2831853) + swirlSpeed * t;
    float radius = swirl * (lifeRatio * (1.0 - lifeRatio) + 0.05) * 0.7; // expand then thin
    pos.x += cos(ang) * radius;
    pos.z += sin(ang) * radius;

    // Small additional jitter for perceived turbulence
    float jitter = (hash2(vec2(id, ubo.time)) - 0.5) * 0.08;
    pos.x += jitter;
    pos.z += jitter * 0.6;

    // Transform to world using model push constant
    vec3 worldPos = (pushConstants.model * vec4(pos, 1.0)).xyz;

    // Clip-space transform
    gl_Position = ubo.proj * ubo.view * vec4(worldPos, 1.0);

    // Point size: scale by distance and age, guard division
    float viewDepth = length((ubo.view * vec4(worldPos, 1.0)).xyz);
    float sizeBase = 200.0; // tweak to taste
    float size = (sizeBase / max(viewDepth, 0.0001)) * (1.0 - lifeRatio * 0.9);
    // Embers are smaller but brighter
    if (vSpark > 0.5) size *= 0.6;
    gl_PointSize = clamp(size, 1.5, 40.0);

    // Color: strong red palette (young->bright, old->dark red)
    if (vSpark > 0.5) {
        // ember core slightly more yellow-white but still red-biased
        vColor = vec3(1.0, 0.6, 0.3);
    } else {
        // red-focused gradient
        if (lifeRatio < 0.3) {
            // bright red-yellowish
            vColor = mix(vec3(1.0, 0.6, 0.3), vec3(1.0, 0.35, 0.15), lifeRatio / 0.3);
        } else if (lifeRatio < 0.6) {
            // mid-life: saturated red-orange
            vColor = mix(vec3(1.0, 0.35, 0.15), vec3(0.95, 0.15, 0.06), (lifeRatio - 0.3) / 0.3);
        } else {
            // old: deep red -> near black red
            vColor = mix(vec3(0.95, 0.15, 0.06), vec3(0.45, 0.02, 0.01), (lifeRatio - 0.6) / 0.4);
        }
    }
}