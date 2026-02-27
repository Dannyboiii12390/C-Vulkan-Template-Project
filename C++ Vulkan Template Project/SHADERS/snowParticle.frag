#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) flat in int isVisible; // 'flat' required for integer varyings
layout(location = 0) out vec4 outColor;

void main() {
    // Convert to centered coordinates [-0.5, 0.5]
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);

    // Soft circular snowflake sprite:
    // - full intensity near center, smooth falloff to edge
    // - discard outside point to keep round shape
    float radius = 0.5;
    if (dist > radius) {
        discard;
    }

    // smooth edge anti-alias
    float edge0 = radius * 0.85;
    float edge = smoothstep(radius, edge0, dist);

    // subtle inner glow to give a fluffy look
    float inner = 1.0 - pow(dist / edge0, 2.0);

    float finalAlpha = fragColor.a * edge * inner;

    // Slight bluish tint and soft premultiplied alpha
    vec3 col = fragColor.rgb;

    outColor = vec4(col * finalAlpha, finalAlpha);

    // If completely transparent, or flagged as not visible (0), discard
    if (outColor.a <= 0.001 || isVisible == 0) {
        discard;
    }

}