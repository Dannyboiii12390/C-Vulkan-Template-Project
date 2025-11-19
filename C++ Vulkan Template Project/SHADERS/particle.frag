#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    // Create circular point sprite
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    
    if (dist > 0.5) {
        discard;  // Make points circular
    }
    
    // Fade out at edges
    float alpha = 1.0 - (dist * 2.0);
    outColor = vec4(fragColor.rgb, fragColor.a * alpha);
}