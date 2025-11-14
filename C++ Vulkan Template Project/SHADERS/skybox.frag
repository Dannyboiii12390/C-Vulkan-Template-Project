#version 450

layout(location = 0) in vec3 fragTexCoord;

layout(binding = 1) uniform samplerCube skySampler;

layout(location = 0) out vec4 outColor;

void main() {
    // Sample the cubemap using the interpolated direction
    outColor = texture(skySampler, fragTexCoord);
}