#version 450

// Push constants (model matrix + optional tex index kept for compatibility)
layout(push_constant) uniform PushConstants {
    mat4 model;
    int texIndex; // keep existing size/layout compatible with pipeline
} pushConstants;

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

// Height map sampler (same binding as fragment shader)
layout(binding = 2) uniform sampler2D heightMap;

// Vertex inputs - must match your Vertex layout (pos, color, normal, texCoord, tangent, binormal)
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBinormal;

// Outputs to fragment shader
layout(location = 0) out vec2 vUV;
layout(location = 1) out vec3 vFragPos;
layout(location = 2) out vec3 vTangent;
layout(location = 3) out vec3 vBitangent;
layout(location = 4) out vec3 vNormal;
layout(location = 5) out vec3 vLightDirTangent_sun;
layout(location = 6) out vec3 vLightDirTangent_moon;
layout(location = 7) out vec3 vViewDirTangent;

// Displacement / bump tuning constants (tweak these to control vertex displacement)
const float heightScale = 2.0;        // overall displacement magnitude (recommended 0.02 - 0.15)
const float displacementScale = 1.0;   // global multiplier for displacement
const float displacementBlend = 1.0;   // 0 = no displacement, 1 = full displacement
const float bumpContrast = 1.0;        // >1 sharpens heights, <1 softens (0.5 - 2.0)
const float heightBias = 0.2;          // biases sampled height before contrast (-0.2 .. 0.2)

// Small offset to sample neighboring texels for gradient estimation.
// Tweak this to match your height map resolution (smaller = higher-res sampling).
const float sampleOffset = 1.0 / 512.0;

void main()
{
    // Transform position and normal/tangent/bitangent to world space
    vec3 worldPos = (pushConstants.model * vec4(inPosition, 1.0)).xyz;
    vec3 worldNormal = normalize((pushConstants.model * vec4(inNormal, 0.0)).xyz);
    vec3 worldTangent = normalize((pushConstants.model * vec4(inTangent, 0.0)).xyz);
    vec3 worldBitangent = normalize((pushConstants.model * vec4(inBinormal, 0.0)).xyz);

    // Sample height and apply bias/contrast
    float height = texture(heightMap, inTexCoord).r;
    height = clamp(pow(height + heightBias, max(bumpContrast, 0.0001)), 0.0, 1.0);

    // Compute displaced world position (displace along world normal)
    vec3 displacedWorldPos = worldPos + worldNormal * (height * heightScale * displacementScale * displacementBlend);

    // Build TBN matrix (columns are tangent, bitangent, normal)
    mat3 TBN = mat3(worldTangent, worldBitangent, worldNormal);

    // Transform light and view vectors into tangent space and pass to fragment shader.
    // We pass the vector from surface to light (so its length is distance).
    vLightDirTangent_sun = TBN * (ubo.sun_pos - displacedWorldPos);
    vLightDirTangent_moon = TBN * (ubo.moon_pos - displacedWorldPos);
    vViewDirTangent = TBN * (ubo.eyePos - displacedWorldPos);

    // Set varyings
    vUV = inTexCoord;
    vNormal = worldNormal;
    vFragPos = displacedWorldPos;
    vTangent = worldTangent;
    vBitangent = worldBitangent;

    // Final clip-space position
    gl_Position = ubo.proj * ubo.view * vec4(displacedWorldPos, 1.0);
}