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

// Varyings from vertex shader
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldNormal; // unused for tangent-space bumping but kept for compatibility
layout(location = 2) in vec3 fragWorldPos;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in vec3 fragLightPos_tangent;
layout(location = 5) in vec3 fragViewPos_tangent;

layout(set = 0, binding = 1) uniform sampler2D albedoTextures[2];
layout(set = 0, binding = 2) uniform sampler2D heightTextures[2];

layout(location = 0) out vec4 outColor;

const float GAMMA = 2.2;

// simple quadratic attenuation
float attenuation(float dist) {
    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;
    return 1.0 / (constant + linear * dist + quadratic * dist * dist);
}

void main() {
    // Select the correct albedo and height map using push constant index
    int idx = clamp(pushConstants.texIndex, 0, 1);
    vec3 albedo = texture(albedoTextures[idx], fragTexCoord).rgb;

    // Sample height map (grayscale stored in .r)
    float h = texture(heightTextures[idx], fragTexCoord).r;

    // Compute derivatives of the sampled height across the screen
    float dFx = dFdx(h);
    float dFy = dFdy(h);

    // Bump parameter (tweak between ~0.1 and 1.0 for stronger/weaker bumps)
    const float bumpHeight = 0.5;

    // Reconstruct normal in TANGENT space from height derivatives
    // Note: the sign convention (-dFx, -dFy, bumpHeight) follows the standard bump/height -> normal trick
    vec3 N_tangent = normalize(vec3(-dFx, -dFy, bumpHeight));

    // Tangent-space light & view vectors are provided by the vertex shader
    vec3 L = normalize(fragLightPos_tangent);     // tangent-space light vector (may encode distance)
    vec3 V = normalize(fragViewPos_tangent);      // tangent-space view vector
    vec3 H = normalize(L + V);

    // Material properties
    float shininess = 32.0;
    float specularStrength = 0.5;
    float metalness = 0.0;
    vec3 ambientColor = vec3(0.03);

    // Diffuse (Blinn-Phong)
    float NdotL = max(dot(N_tangent, L), 0.0);
    vec3 diffuse = albedo * NdotL;

    // Specular
    float NdotH = max(dot(N_tangent, H), 0.0);
    float specFactor = pow(NdotH, shininess);
    vec3 specularBase = mix(vec3(0.04), albedo, metalness);
    vec3 specular = specularStrength * specFactor * specularBase;

    // Distance-based attenuation (fragLightPos_tangent length used as approximate distance)
    float dist = length(fragLightPos_tangent);
    float att = attenuation(dist);
    diffuse *= att;
    specular *= att;

    vec3 ambient = ambientColor * albedo;
    vec3 resultLinear = ambient + diffuse + specular;

    // Gamma correction
    vec3 resultGamma = pow(resultLinear, vec3(1.0 / GAMMA));
    //outColor = vec4(resultGamma, 1.0);
}