#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 0) out vec4 outColor;
layout(location = 1) flat in int isVisible; // 'flat' required for integer varyings

void main()
{
    // Make circular point sprite
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5) discard;

    // Soft radial falloff so particles blend nicely
    float edge = smoothstep(0.5, 0.0, dist);
    // Slightly sharpen center to avoid very fuzzy cloud
    float centerBoost = smoothstep(0.25, 0.0, dist) * 0.35;
    float alpha = fragColor.a * edge + centerBoost * fragColor.a;

    outColor = vec4(fragColor.rgb, alpha);

    // Discard fully transparent fragments
    if (outColor.a <= 0.001 || isVisible == 0) discard;
}