#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
    int texIndex;
} pushConstants;

layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 lightPos;
    vec4 redLightPos;
    vec4 eyePos;
} ubo;

// Optional varying inputs kept for compatibility
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldNormal;
layout(location = 2) in vec3 fragWorldPos;

// New inputs required for tangent-space lighting
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in vec3 fragLightPos_tangent; // provided in tangent-space (vector from frag -> light)
layout(location = 5) in vec3 fragViewPos_tangent;  // provided in tangent-space (vector from frag -> view/cam)

// Samplers
layout(binding = 1) uniform sampler2D colSampler;
layout(binding = 2) uniform sampler2D normalSampler;

layout(location = 0) out vec4 outColor;

const float GAMMA = 2.2;

float attenuation(float dist) {
    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;
    return 1.0 / (constant + linear * dist + quadratic * dist * dist);
}

void main() {
    // Sample base color and normal (normal map encoded in [0,1])
    vec3 albedo = texture(colSampler, fragTexCoord).rgb;
    vec3 N_tangent = texture(normalSampler, fragTexCoord).rgb;
    N_tangent = normalize(N_tangent * 2.0 - 1.0);

    // Assumption: fragLightPos_tangent and fragViewPos_tangent are tangent-space direction vectors
    vec3 L = normalize(fragLightPos_tangent);
    vec3 V = normalize(fragViewPos_tangent);
    vec3 H = normalize(L + V);

    // Material properties
    float shininess = 32.0;
    float specularStrength = 0.5;
    float metalness = 0.0;
    vec3 ambientColor = vec3(0.03);

    // Diffuse
    float NdotL = max(dot(N_tangent, L), 0.0);
    vec3 diffuse = albedo * NdotL;

    // Specular (Blinn-Phong)
    float NdotH = max(dot(N_tangent, H), 0.0);
    float specFactor = pow(NdotH, shininess);
    vec3 specularBase = mix(vec3(0.04), albedo, metalness);
    vec3 specular = specularStrength * specFactor * specularBase;

    // Simple distance-based attenuation if the provided L/V also encode distance (optional)
    // If fragLightPos_tangent is a direction only, distance-based attenuation can be skipped.
    float dist = length(fragLightPos_tangent);
    float att = attenuation(dist);
    diffuse *= att;
    specular *= att;

    vec3 ambient = ambientColor * albedo;

    vec3 resultLinear = ambient + diffuse + specular;
    vec3 resultGamma = pow(resultLinear, vec3(1.0 / GAMMA));

    outColor = vec4(resultGamma, 1.0);
}