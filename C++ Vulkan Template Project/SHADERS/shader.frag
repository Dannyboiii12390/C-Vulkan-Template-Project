#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
} pushConstants;

layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 lightPos;      // static white light position (world)
    vec4 redLightPos;   // dynamic red light position (world)
    vec4 eyePos;        // camera position (world)
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldNormal;
layout(location = 2) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

const float GAMMA = 2.2;

vec3 calcBlinnPhong(vec3 N, vec3 V, vec3 L, vec3 lightColor, float intensity, vec3 albedo, float shininess, float specularStrength, float metalness) {
    vec3 H = normalize(L + V);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    vec3 diffuse = albedo * NdotL;
    float specFactor = pow(NdotH, shininess);
    vec3 specular = specularStrength * specFactor * mix(vec3(0.04), albedo, metalness);

    return lightColor * intensity * (diffuse + specular);
}

float attenuation(float dist) {
    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;
    return 1.0 / (constant + linear * dist + quadratic * dist * dist);
}

void main() {
    vec3 N = normalize(fragWorldNormal);
    vec3 V = normalize(ubo.eyePos.xyz - fragWorldPos);

    vec3 whitePos = ubo.lightPos.xyz;
    vec3 redPos = ubo.redLightPos.xyz;

    vec3 whiteColor = vec3(1.0);
    vec3 redColor   = vec3(1.0, 0.0, 0.0);

    float whiteIntensity = 1.0;
    float redIntensity   = 1.0;

    float modelX = pushConstants.model[3].x;

    vec3 albedo = fragColor;
    float shininess = 32.0;
    float specularStrength = 0.5;
    float metalness = 0.0;
    vec3 ambientColor = vec3(0.03);

    if (modelX < -0.5) {
        albedo = fragColor * vec3(0.9, 0.6, 0.6);
        shininess = 8.0;
        specularStrength = 0.15;
        metalness = 0.0;
    } else if (modelX < 0.5) {
        albedo = fragColor * vec3(0.4, 0.7, 0.4);
        shininess = 64.0;
        specularStrength = 1.0;
        metalness = 0.0;
    } else {
        albedo = fragColor * vec3(0.9, 0.9, 0.95);
        shininess = 128.0;
        specularStrength = 0.9;
        metalness = 1.0;
    }

    vec3 Lw = normalize(whitePos - fragWorldPos);
    float distW = length(whitePos - fragWorldPos);
    float attW = attenuation(distW);
    vec3 whiteContrib = calcBlinnPhong(N, V, Lw, whiteColor, whiteIntensity * attW, albedo, shininess, specularStrength, metalness);

    vec3 Lr = normalize(redPos - fragWorldPos);
    float distR = length(redPos - fragWorldPos);
    float attR = attenuation(distR);
    vec3 redContrib = calcBlinnPhong(N, V, Lr, redColor, redIntensity * attR, albedo, shininess, specularStrength, metalness);

    vec3 colorLinear = ambientColor * albedo + whiteContrib + redContrib;
    vec3 colorGamma = pow(colorLinear, vec3(1.0 / GAMMA));
    outColor = vec4(colorGamma, 1.0);
}