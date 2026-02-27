#version 450

layout(location = 0) in float vLifeRatio;
layout(location = 1) in float vSpark;
layout(location = 2) in vec3 vColor;

layout(location = 0) out vec4 outColor;

void main() {
    // Make point sprites circular
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5) discard;

    // Soft radial falloff
    float falloff = smoothstep(0.5, 0.0, dist);

    // Edge softening for smoother blending
    float edge = smoothstep(0.45, 0.5, dist);
    float alpha = (1.0 - vLifeRatio) * falloff * (1.0 - edge * 0.9);

    // Force a red bias and slightly boost brightness for younger particles
    vec3 redTint = vColor;
    // Slightly enhance red channel for strong red look
    redTint.r = clamp(redTint.r * 1.15, 0.0, 1.0);
    redTint.g *= 0.6;
    redTint.b *= 0.4;

    if (vSpark > 0.5) {
        // ember core + glow, red-biased
        float core = smoothstep(0.25, 0.0, dist);
        vec3 coreCol = vec3(1.0, 0.75, 0.45); // bright core
        vec3 glowCol = redTint;
        outColor.rgb = mix(glowCol, coreCol, core);
        outColor.a = clamp(alpha * 2.2 + core * 0.6, 0.0, 1.0);
    } else {
        // Regular red flame: use redTint and falloff
        // Boost overall red intensity for young particles
        float brightness = 0.8 + 0.6 * (1.0 - vLifeRatio);
        outColor = vec4(redTint * brightness, alpha);
    }

    // Optional: slightly desaturate as particles age (subtle)
    float desat = mix(0.0, 0.6, vLifeRatio);
    vec3 gray = vec3(dot(outColor.rgb, vec3(0.299, 0.587, 0.114)));
    outColor.rgb = mix(outColor.rgb, gray, desat * 0.2);
}