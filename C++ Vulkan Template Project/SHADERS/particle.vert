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
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec4 fragColor;

// Simple hash function for pseudo-random values based on particle ID
float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

void main() {
    float particleId = float(gl_VertexIndex);
    
    // Generate pseudo-random values for this particle
    float randomAngle = hash(particleId) * 6.28318;  // 0 to 2*PI
    float randomSpeed = 0.5 + hash(particleId + 100.0) * 1.5;
    float randomHeight = hash(particleId + 200.0) * 2.0;
    float randomLifetime = hash(particleId + 300.0) * 3.0;
    
    // Calculate lifetime (loop every 3 seconds)
    float lifetime = mod(ubo.time + randomLifetime, 3.0);
    float lifeRatio = lifetime / 3.0;
    
    // Animate position
    vec3 animatedPos = inPosition;
    
    // Upward motion with gravity
    animatedPos.y += lifetime * 2.0 - lifetime * lifetime * 0.5;
    
    // Horizontal spread
    animatedPos.x += cos(randomAngle) * randomSpeed * lifetime;
    animatedPos.z += sin(randomAngle) * randomSpeed * lifetime;
    
    // Transform to clip space
    gl_Position = ubo.proj * ubo.view * vec4(animatedPos, 1.0);
    
    // Calculate point size based on distance and lifetime
    float distance = length((ubo.view * vec4(animatedPos, 1.0)).xyz);
    float sizeFade = 1.0 - lifeRatio;  // Shrink as particle ages
    gl_PointSize = clamp((300.0 / distance) * sizeFade, 0.0, 20.0);
    
    // Color based on lifetime (fire gradient)
    if (lifeRatio < 0.3) {
        // Young: white -> yellow
        fragColor = mix(vec4(1.0, 1.0, 1.0, 1.0), vec4(1.0, 0.8, 0.2, 1.0), lifeRatio / 0.3);
    } else if (lifeRatio < 0.6) {
        // Middle: yellow -> orange
        fragColor = mix(vec4(1.0, 0.8, 0.2, 1.0), vec4(1.0, 0.3, 0.0, 1.0), (lifeRatio - 0.3) / 0.3);
    } else {
        // Old: orange -> red, fade out
        vec4 red = vec4(0.5, 0.0, 0.0, 0.0);
        vec4 orange = vec4(1.0, 0.3, 0.0, 1.0);
        fragColor = mix(orange, red, (lifeRatio - 0.6) / 0.4);
    }
}