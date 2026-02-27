#version 450

// Add explicit precision to reduce compiler variance
precision highp float;

layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;
layout(location = 3) out vec3 vTangent;
layout(location = 4) out vec3 vBitangent;
layout(location = 5) out vec3 vLighting; // Pass lighting to fragment

// Simple diffuse calculator
vec3 CalcDiffuse(vec3 lightPos, vec3 lightColor, float intensity, vec3 worldPos, vec3 normal) {
    vec3 L = normalize(lightPos - worldPos);
    float NdotL = max(dot(normal, L), 0.0);
    // Ensure consistent order of operations
    return (lightColor * intensity) * NdotL;
}

// Maps light world Y to linear strength: y = 0 -> 0.0, y = 200 -> 1.0, clamped.
// Adjust baseline or range if your terrain baseline isn't y=0.
float HeightStrength(vec3 lightPos) {
    // Use const to prevent optimization differences
    const float HEIGHT_SCALE = 1.0 / 200.0;
    return clamp(lightPos.y * HEIGHT_SCALE, 0.0, 1.0);
}

void main()
{
    // Transform position to world space
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    vWorldPos = worldPos.xyz;

    // Transform normal to world space - use explicit mat3 to ensure consistent extraction
    mat3 normalMatrix = mat3(pc.model);
    vNormal = normalize(normalMatrix * inNormal);

    vTexCoord = inTexCoord;
    vTangent = normalize(normalMatrix * inTangent);
    vBitangent = normalize(normalMatrix * inBitangent);

    // Compute per-light height strength (200 -> 100%, 0 -> 0%)
    float sunHeightFactor = HeightStrength(ubo.sun_pos);
    float moonHeightFactor = HeightStrength(ubo.moon_pos);

    // Initialize with explicit zeros to ensure consistency
    vec3 sunLight = vec3(0.0, 0.0, 0.0);
    vec3 moonLight = vec3(0.0, 0.0, 0.0);

    // Use consistent epsilon for comparisons
    const float EPSILON = 0.001;
    
    if (sunHeightFactor > EPSILON) {
        sunLight = CalcDiffuse(ubo.sun_pos, ubo.sun_color, ubo.sun_intensity * sunHeightFactor, vWorldPos, vNormal);
    }

    if (moonHeightFactor > EPSILON) {
        moonLight = CalcDiffuse(ubo.moon_pos, ubo.moon_color, ubo.moon_intensity * moonHeightFactor, vWorldPos, vNormal);
    }

    // Combine contributions. Both can add together if both above terrain.
    vec3 lightAccum = sunLight + moonLight;

    // Small ambient scaled by visible lights' height factors to avoid completely black surfaces.
    vec3 ambientFromLights = vec3(0.03) * (ubo.sun_color * sunHeightFactor + ubo.moon_color * moonHeightFactor);
    lightAccum += ambientFromLights;

    // Guaranteed tiny ambient so surfaces never fully black, plus a stronger fallback when both lights are inactive.
    const float guaranteedAmbient = 0.025; // always present
    const float fallbackAmbient = 0.02;    // added when both lights are fully inactive
    lightAccum += guaranteedAmbient;

    if (sunHeightFactor <= EPSILON && moonHeightFactor <= EPSILON) {
        lightAccum += fallbackAmbient;
    }

    vLighting = lightAccum;

    // Final position
    gl_Position = ubo.proj * ubo.view * worldPos;
}