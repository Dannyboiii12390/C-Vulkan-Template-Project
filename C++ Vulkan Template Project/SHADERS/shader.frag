#version 450

// Input from vertex shader
layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inWorldPos;
layout(location = 2) in vec3 inWorldNormal;
// NEW: object-space values provided by vertex shader to avoid inverse() in fragment
layout(location = 3) in vec3 inSunPosObj;
layout(location = 4) in vec3 inMoonPosObj;
layout(location = 5) in vec3 inNormalObj;

// Output
layout(location = 0) out vec4 outColor;

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

// Albedo textures (array of 2)
layout(binding = 1) uniform sampler2D albedoSamplers[2];

// Normal textures (array of 2)
layout(binding = 2) uniform sampler2D normalSamplers[2];

// Skybox cubemap for reflections/refractions
layout(binding = 3) uniform samplerCube skySampler;

// Push constants
layout(push_constant) uniform PushConstants {
    mat4 model;
    int texIndex;
} pushConstants;

// fast angular mask using dot-product (avoids acos)
float angularMaskDot(vec3 dir, vec3 lightDir, float radiusRad, float glowRad)
{
    vec3 d = normalize(dir);
    vec3 l = normalize(lightDir);
    float cosAng = clamp(dot(d, l), -1.0, 1.0);
    // convert angular radius -> cos thresholds
    float cosR = cos(radiusRad);
    float cosRG = cos(radiusRad + glowRad);
    // smoothstep from outer (cosRG) -> inner (cosR)
    return smoothstep(cosRG, cosR, cosAng);
}

void main() {
     // world-space normal & view
     vec3 N = normalize(inWorldNormal);
     vec3 V = normalize(ubo.eyePos - inWorldPos);

     // --- sky/refraction (existing) ---
     float IOR = 1.00 / 1.33; // Index of Refraction (air to water)
     vec3 R = refract(-V, N, IOR);
     vec3 refractionColor = texture(skySampler, R).rgb;

     // --- Sun & Moon directions in world space (existing) ---
     vec3 sunDir = normalize(ubo.sun_pos - ubo.eyePos);
     vec3 moonDir = normalize(ubo.moon_pos - ubo.eyePos);

     // Apparent angular sizes (degrees, tweakable)
     float sunRadiusDeg  = 0.6;
     float sunGlowDeg    = 2.2;
     float moonRadiusDeg = 0.45;
     float moonGlowDeg   = 1.6;

     const float DEG_TO_RAD = 3.14159265358979323846 / 180.0;
     float sunRadius = sunRadiusDeg * DEG_TO_RAD;
     float sunGlow = sunGlowDeg * DEG_TO_RAD;
     float moonRadius = moonRadiusDeg * DEG_TO_RAD;
     float moonGlow = moonGlowDeg * DEG_TO_RAD;

     // Mask values for sky/refraction (1.0 at center -> 0.0 outside halo)
     float sunMaskSky = angularMaskDot(R, sunDir, sunRadius, sunGlow);
     float moonMaskSky = angularMaskDot(R, moonDir, moonRadius, moonGlow);

     // Visibility factor based on height above horizon (simple)
     float sunVisible = clamp(ubo.sun_pos.y / 200.0, 0.0, 1.0);
     float moonVisible = clamp(ubo.moon_pos.y / 200.0, 0.0, 1.0);

     // Shape core/halo for sky
     float sunCoreSky = pow(sunMaskSky, 0.7);
     float sunHaloSky = pow(sunMaskSky, 1.4) * 0.6;
     float moonCoreSky = pow(moonMaskSky, 0.9);
     float moonHaloSky = pow(moonMaskSky, 1.8) * 0.35;

     vec3 sunEmitSky = ubo.sun_color * ubo.sun_intensity * (sunCoreSky + sunHaloSky) * sunVisible;
     vec3 moonEmitSky = ubo.moon_color * ubo.moon_intensity * (moonCoreSky + moonHaloSky) * moonVisible;

     // --- Paint sun & moon onto the globe's albedo texture ---
     // Sample base albedo
     vec3 albedo = texture(albedoSamplers[pushConstants.texIndex], inTexCoord).rgb;

     // Use object-space values provided by vertex shader (avoid per-fragment inverse)
     vec3 sunPosObj = inSunPosObj;
     vec3 moonPosObj = inMoonPosObj;
     vec3 Nobj = normalize(inNormalObj);

     // Directions from object center to lights in object space
     vec3 LsunObj = normalize(sunPosObj);
     vec3 LmoonObj = normalize(moonPosObj);

     // Angular masks on the globe surface (object-space)
     float sunMaskTex = angularMaskDot(Nobj, LsunObj, sunRadius, sunGlow);
     float moonMaskTex = angularMaskDot(Nobj, LmoonObj, moonRadius, moonGlow);

     // Slightly different shaping for texture painting
     float sunCoreTex = pow(sunMaskTex, 0.6);
     float sunHaloTex = pow(sunMaskTex, 1.6) * 0.7;
     float moonCoreTex = pow(moonMaskTex, 0.8);
     float moonHaloTex = pow(moonMaskTex, 2.0) * 0.45;

     // Emissive contributions painted onto the albedo (scale down if needed)
     vec3 sunEmitTex = ubo.sun_color * ubo.sun_intensity * (sunCoreTex + sunHaloTex) * sunVisible * 0.9;
     vec3 moonEmitTex = ubo.moon_color * ubo.moon_intensity * (moonCoreTex + moonHaloTex) * moonVisible * 0.7;

     // Add the painted highlights onto the albedo
     vec3 paintedAlbedo = albedo + sunEmitTex + moonEmitTex;

     // Combine painted albedo with refraction/sky contribution (small blend so globe texture remains primary)
     vec3 composed = paintedAlbedo * 0.92 + refractionColor * 0.08 + sunEmitSky + moonEmitSky;

     // Tone mapping / clamp
     composed = composed / (1.0 + composed);

     outColor = vec4(clamp(composed, 0.0, 1.0), 1.0);
}