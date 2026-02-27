#version 450

// Push constants (matches vertex)
layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

// UBO (binding = 0) - must match application layout
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

// Inputs from vertex shader (locations must match)
layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vFragPos;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vNormal;

// Output
layout(location = 0) out vec4 outColor;

// Textures (bindings — adjust descriptor set in host code if different)
layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D heightMap;

// Simple parameters
const float ambientStrength = 0.06;
const float specularStrength = 1.0;
const float shininess = 64.0;

// Controls bump intensity (tweak to taste; start small)
const float heightScale = 0.04;

// Calculates ambient+diffuse; returns specular through out parameter
vec3 CalcLightBase(vec3 lightPos, vec3 lightColor, float intensity, vec3 N, vec3 V, vec3 fragPos, out vec3 outSpec)
{
    vec3 L = normalize(lightPos - fragPos);
    float diff = max(dot(N, L), 0.0);

    // Blinn-Phong half vector
    vec3 H = normalize(L + V);
    float specFactor = pow(max(dot(N, H), 0.0), shininess);

    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = diff * lightColor;
    vec3 specular = specularStrength * specFactor * lightColor;

    outSpec = intensity * specular;
    return intensity * (ambient + diffuse);
}

// Maps light world Y to linear strength: y = 0 -> 0.0, y = 200 -> 1.0, clamped.
// Adjust baseline or range if your terrain baseline isn't y=0.
float HeightStrength(vec3 lightPos) {
    return clamp(lightPos.y / 200.0, 0.0, 1.0);
}

void main()
{
    // Sample albedo (assumed linear because image view = SRGB)
    vec4 albedoSample = texture(albedoMap, vUV);
    vec3 albedo = albedoSample.rgb;

    // Bump mapping: central differences in UV space
    ivec2 texDim = textureSize(heightMap, 0);
    vec2 texSize = vec2(max(texDim, ivec2(1)));
    vec2 texel = 1.0 / texSize;

    vec2 uv = clamp(vUV, vec2(0.0), vec2(1.0));
    float hc = texture(heightMap, uv).r;
    float hl = texture(heightMap, clamp(uv - vec2(texel.x, 0.0), 0.0, 1.0)).r;
    float hr = texture(heightMap, clamp(uv + vec2(texel.x, 0.0), 0.0, 1.0)).r;
    float hd = texture(heightMap, clamp(uv - vec2(0.0, texel.y), 0.0, 1.0)).r;
    float hu = texture(heightMap, clamp(uv + vec2(0.0, texel.y), 0.0, 1.0)).r;

    // Central difference in UV space (do NOT divide by texel; hr-hl is in UV space)
    float dHdU = (hr - hl) * 0.5;
    float dHdV = (hu - hd) * 0.5;

    // Tangent-space normal from height derivatives
    vec3 tangentSpaceNormal = normalize(vec3(-dHdU * heightScale, -dHdV * heightScale, 1.0));

    // Reconstruct TBN from interpolated vertex data (already orthonormalized in vertex shader)
    vec3 T = normalize(vTangent);
    vec3 B = normalize(vBitangent);
    vec3 N = normalize(vNormal);
    mat3 TBN = mat3(T, B, N); // columns: tangent, bitangent, normal

    // Convert normal from tangent space to world space
    vec3 N_world = normalize(TBN * tangentSpaceNormal);

    // View vector
    vec3 V = normalize(ubo.eyePos - vFragPos);

    // Compute height-based strength for each light (200 -> 100%, 0 -> 0%)
    float sunHeightFactor = HeightStrength(ubo.sun_pos);
    float moonHeightFactor = HeightStrength(ubo.moon_pos);

    // Accumulate lighting from sun and moon, separating specular so it is not modulated by albedo
    vec3 baseLighting = vec3(0.0);
    vec3 specularAccum = vec3(0.0);
    vec3 specOut;

    if (sunHeightFactor > 0.0) {
        baseLighting += CalcLightBase(ubo.sun_pos, ubo.sun_color, ubo.sun_intensity * sunHeightFactor, N_world, V, vFragPos, specOut);
        specularAccum += specOut;
    }

    if (moonHeightFactor > 0.0) {
        baseLighting += CalcLightBase(ubo.moon_pos, ubo.moon_color, ubo.moon_intensity * moonHeightFactor, N_world, V, vFragPos, specOut);
        specularAccum += specOut;
    }

    // Guaranteed tiny ambient so terrain never fully black, plus a stronger fallback when both lights are inactive.
    const float guaranteedAmbient = 0.025; // always present
    const float fallbackAmbient = 0.02;    // added when both lights are fully inactive
    baseLighting += guaranteedAmbient * vec3(1.0);

    if (sunHeightFactor <= 0.0 && moonHeightFactor <= 0.0) {
        baseLighting += fallbackAmbient * vec3(1.0);
    }

    // Combine: albedo modulates ambient+diffuse, specular is added on top (no manual gamma here)
    vec3 colorLinear = albedo * baseLighting + specularAccum;

    // Clamp in linear space (no gamma correction — swapchain is assumed sRGB)
    vec3 color = clamp(colorLinear, 0.0, 1.0);

    outColor = vec4(color, albedoSample.a);
}