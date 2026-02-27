#version 450

layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;
layout(location = 5) in vec3 vLighting; // Receive lighting

layout(location = 0) out vec4 outColor;

// Example: texture sampler (binding = 1)
layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    vec3 baseColor = texture(texSampler, vTexCoord).rgb;
    vec3 litColor = baseColor * vLighting;
    outColor = vec4(litColor, 1.0);
}