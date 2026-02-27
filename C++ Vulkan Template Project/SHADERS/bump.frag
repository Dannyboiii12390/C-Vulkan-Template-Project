#version 450

// Push constants
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
    mat4 lightSpaceMatrix;
} ubo;

// Inputs from vertex shader
layout(location = 0) in vec2 vUV;
layout(location = 1) in vec3 vFragPos;
layout(location = 2) in vec3 vTangent;
layout(location = 3) in vec3 vBitangent;
layout(location = 4) in vec3 vNormal;
layout(location = 5) in vec3 vLightDirTangent_sun;
layout(location = 6) in vec3 vLightDirTangent_moon;
layout(location = 7) in vec3 vViewDirTangent;

// Output
layout(location = 0) out vec4 outColor;

// TEXTURE SAMPLERS
layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D heightMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2DShadow shadowMap;

// Constants
const float heightScale = 0.0; // parallax disabled by default
const float bumpContrast = 2.0;
const float specularStrength = 0.6;
const float shininess = 48.0;

// Shadow calculation
float calculateShadow(sampler2DShadow shadowSampler, vec4 fragPosLightSpace, vec3 normalWorld, vec3 lightDirWorld) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    float z = projCoords.z;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        z < 0.0 || z > 1.0) {
        return 0.0;
    }

    // bias uses world-space normal and world-space lightDir
    float NdotL = max(dot(normalWorld, lightDirWorld), 0.0);
    float bias = max(0.002 * (1.0 - NdotL), 0.0005);

    // 3x3 PCF
    vec2 texelSize = 1.0 / vec2(textureSize(shadowSampler, 0));
    float visibility = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            visibility += texture(shadowSampler, vec3(projCoords.xy + vec2(x, y) * texelSize, z - bias));
        }
    }
    visibility /= 9.0;
    return 1.0 - visibility;
}

// Parallax mapping (kept intact)
vec2 parallaxMapping(vec2 texCoords, vec3 viewDir) {
    if (heightScale <= 0.0) return texCoords;

    const float minLayers = 8.0;
    const float maxLayers = 16.0;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    float layerDepth = 1.0 / numLayers;
    vec2 deltaTexCoords = viewDir.xy * heightScale / numLayers;

    vec2 currentTexCoords = texCoords;
    float currentLayerDepth = 0.0;
    float currentDepthMapValue = texture(heightMap, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue && currentLayerDepth < 1.0) {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(heightMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
    return clamp(finalTexCoords, 0.0, 1.0);
}

void main() {
    // Parallax (if enabled)
    vec2 texCoords = parallaxMapping(vUV, normalize(vViewDirTangent));

    // Sample maps
    vec3 albedo = texture(albedoMap, texCoords).rgb;
    vec3 normalTangent = normalize(texture(normalMap, texCoords).rgb * 2.0 - 1.0);
    normalTangent = normalize(normalTangent * bumpContrast);

    // Build TBN from interpolated vertex TBN (vertex shader produced world-space T/B/N)
    mat3 TBN = mat3(normalize(vTangent), normalize(vBitangent), normalize(vNormal));

    // world-space normal used only for shadow computations and bias
    vec3 normalWorld = normalize(TBN * normalTangent);

    // Shadow coords from light-space matrix (light-space must be consistent with shadow pass)
    vec4 fragPosLightSpace = ubo.lightSpaceMatrix * vec4(vFragPos, 1.0);

    // World-space light direction for shadows (use position -> frag if sun_pos is a world-space position)
    // For directional sun at infinity you may supply normalized direction instead; keep CPU/UBO consistent.
    vec3 lightDirWorld_forShadow = normalize(ubo.sun_pos - vFragPos);

    float shadowFactor = calculateShadow(shadowMap, fragPosLightSpace, normalWorld, lightDirWorld_forShadow);

    // --- Tangent-space lighting (keep bump mapping) ---
    vec3 L_tangent_sun = normalize(vLightDirTangent_sun);   // passed from vertex shader (TBN*(sun_pos - pos))
    vec3 V_tangent = normalize(vViewDirTangent);

    float diff_sun = max(dot(normalTangent, normalize(L_tangent_sun)), 0.0);

    vec3 halfDir = normalize(normalize(L_tangent_sun) + V_tangent);
    float spec = pow(max(dot(normalTangent, halfDir), 0.0), shininess);

    // Sun intensity ramp by height (your requirement kept)
    float sunHeightFactor = clamp(ubo.sun_pos.y / 50.0, 0.0, 1.0);
    float sunIntensity = ubo.sun_intensity * sunHeightFactor;

    vec3 ambient = vec3(0.15) * albedo;
    vec3 diffuse = diff_sun * sunIntensity * (1.0 - shadowFactor * 0.5) * ubo.sun_color * albedo;
    vec3 specular = spec * sunIntensity * (1.0 - shadowFactor * 0.5) * ubo.sun_color * specularStrength;

    vec3 color = ambient + diffuse + specular;
    outColor = vec4(color, 1.0);
}