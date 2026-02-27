#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) flat in int isVisible; // 'flat' required for integer varyings
layout(location = 0) out vec4 outColor;

void main() {
    // gl_PointCoord: (0..1, 0..1) within the point sprite
    // We shape the sprite to be a vertical streak:
    // - horizontally: keep narrow, smooth edges
    // - vertically: long gradient (bright at top, faded at tail)
    float x = gl_PointCoord.x;
    float y = gl_PointCoord.y;

    // Horizontal shaping (narrow streak)
    float xCenter = 0.5;
    float xDist = abs(x - xCenter);
    float edgeSoftness = 0.08; // softness of horizontal edge
    float halfWidth = 0.25;    // controls overall width of the streak
    if (xDist > halfWidth + edgeSoftness) {
        discard;
    }
    // horizontal alpha profile (1.0 in center -> 0 at edges)
    float hAlpha = 1.0 - smoothstep(halfWidth - edgeSoftness, halfWidth + edgeSoftness, xDist);

    // Vertical fade: bright near the top (where drop front is) and fade towards tail
    // Use a power curve so tail fades quicker.
    float vAlpha = pow(1.0 - y, 1.8); // y==0 top, y==1 bottom

    // Combine alphas and apply incoming vertex alpha
    float finalAlpha = fragColor.a * hAlpha * vAlpha;

    // Slight highlight toward the top of the streak
    float highlight = smoothstep(0.0, 0.12, 1.0 - y);
    vec3 highlightColor = vec3(1.0, 1.0, 1.0) * 0.25 * highlight;

    outColor = vec4(fragColor.rgb + highlightColor, finalAlpha);

    // If completely transparent, or flagged as not visible (0), discard
    if (outColor.a <= 0.001 || isVisible == 0) {
        discard;
    }
}