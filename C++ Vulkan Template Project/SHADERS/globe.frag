#version 450

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vWorldNormal;
layout(location = 2) in vec2 vUV;
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

// Globe albedo texture (binding = 1)
layout(binding = 1) uniform sampler2D uAlbedo;

// (optional) environment cubemap for subtle reflections (binding = 2)
layout(binding = 2) uniform samplerCube uSky;

// Utility: angular mask using dot-product
float angularMaskDot(vec3 dir, vec3 lightDir, float radiusRad, float glowRad)
{
    vec3 d = normalize(dir);
    vec3 l = normalize(lightDir);
    float cosAng = clamp(dot(d, l), -1.0, 1.0);
    float cosR = cos(radiusRad);
    float cosRG = cos(radiusRad + glowRad);
    return smoothstep(cosRG, cosR, cosAng);
}

void main()
{
    vec3 SUN_COLOR = ubo.sun_color;
    vec3 MOON_COLOR = ubo.moon_color;


    // If globe is centred at origin use vWorldPos normalized as surface direction
    vec3 globeCenter = vec3(0.0); // change if globe is not at origin
    vec3 surfaceDir = normalize(vWorldPos - globeCenter); // direction from globe center to this fragment

    // Compute robust spherical UV from world-space direction to avoid interpolation seam
    const float PI = 3.14159265358979323846;
    vec2 sphUV;
    sphUV.x = 0.5 + atan(surfaceDir.z, surfaceDir.x) / (2.0 * PI);
    sphUV.y = 0.5 - asin(surfaceDir.y) / PI;
    sphUV.x = fract(sphUV.x); // ensure wrapping in [0,1)

    // base albedo (globe surface texture) — use spherical UV instead of interpolated mesh UV
    vec3 albedo = texture(uAlbedo, sphUV).rgb;

    // Sun / moon world directions relative to globe center
    vec3 sunDirWorld = normalize(ubo.sun_pos - globeCenter);
    vec3 moonDirWorld = normalize(ubo.moon_pos - globeCenter);

    // Angular sizes in radians (tweak)
    const float DEG_TO_RAD = 3.14159265358979323846 / 180.0;
    float sunRadius  = 1.6 * DEG_TO_RAD;
    float sunGlow    = 3.2 * DEG_TO_RAD;
    float moonRadius = 1.0 * DEG_TO_RAD;
    float moonGlow   = 3.2 * DEG_TO_RAD;

    // Masks: 1 at center -> 0 at outer halo
    float sunMask = angularMaskDot(surfaceDir, sunDirWorld, sunRadius, sunGlow);
    float moonMask = angularMaskDot(surfaceDir, moonDirWorld, moonRadius, moonGlow);

    // Visibility scaling (optional)
    float sunVisible = clamp(ubo.sun_pos.y / 200.0, 0.0, 1.0);
    float moonVisible = clamp(ubo.moon_pos.y / 200.0, 0.0, 1.0);

    // Core and halo shaping
    float sunCore = pow(sunMask, 0.7);
    float sunHalo = pow(sunMask, 1.4) * 0.6;
    float moonCore = pow(moonMask, 0.9);
    float moonHalo = pow(moonMask, 1.8) * 0.35;

    // Emissive contributions (additive - true emission)
    vec3 sunEmit = SUN_COLOR * ubo.sun_intensity * (sunCore + sunHalo) * sunVisible;
    vec3 moonEmit = MOON_COLOR * ubo.moon_intensity * (moonCore + moonHalo) * moonVisible;

    // subtle environment (optional)
    vec3 env = vec3(0.0);
    // compute a world-space view direction for env sampling if uSky is present
    vec3 viewDir = normalize(ubo.eyePos - vWorldPos);
    env = texture(uSky, viewDir).rgb * 0.05; // tiny contribution

    // Combine: albedo modulated by any lighting you want (here we use albedo as base)
    // Emissive sun/moon are additive (so they remain bright regardless of albedo)
    vec3 colorLinear = albedo + env + sunEmit + moonEmit;

    // Simple tone map + gamma-safe clamp
    colorLinear = colorLinear / (1.0 + colorLinear);
    colorLinear = clamp(colorLinear, 0.0, 1.0);

    outColor = vec4(colorLinear, 1.0);
}