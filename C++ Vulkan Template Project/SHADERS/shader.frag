
#version 450

layout(push_constant) uniform PushConstants {
    mat4 model;
    int texIndex;  // 0 = blend mode, 1 = coin only, 2 = tile only
} pushConstants;

layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 lightPos;
    vec4 redLightPos;
    vec4 eyePos;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldNormal;
layout(location = 2) in vec3 fragWorldPos;
layout(location = 3) in vec2 fragTexCoord;

layout(set = 0, binding = 1) uniform sampler2D textures[2];

layout(location = 0) out vec4 outColor;

const float GAMMA = 2.2;

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

    // Sample both textures
    vec4 coinColor = texture(textures[0], fragTexCoord);
    vec4 tileColor = texture(textures[1], fragTexCoord);
    
    // Blend textures based on mode
    vec4 texColor;
    if (pushConstants.texIndex == 0) {
        // Blend mode: multiply coin and tile textures
        texColor = coinColor * tileColor;
    } else if (pushConstants.texIndex == 1) {
        // Coin only
        texColor = coinColor;
    } else {
        // Tile only
        texColor = tileColor;
    }
    
    vec3 albedo = texColor.rgb;
    float shininess = 32.0;
    float specularStrength = 0.5;
    float metalness = 0.0;
    vec3 ambientColor = vec3(0.03);

    // Material properties based on position
    if (modelX < -0.5) {
        albedo = texColor.rgb * vec3(0.9, 0.6, 0.6);
        shininess = 8.0;
        specularStrength = 0.15;
        metalness = 0.0;
    } else if (modelX < 0.5) {
        albedo = texColor.rgb * vec3(0.4, 0.7, 0.4);
        shininess = 64.0;
        specularStrength = 1.0;
        metalness = 0.0;
    } else {
        albedo = texColor.rgb * vec3(0.9, 0.9, 0.95);
        shininess = 128.0;
        specularStrength = 0.9;
        metalness = 1.0;
    }

    // White light contributions
    vec3 Lw = normalize(whitePos - fragWorldPos);
    float distW = length(whitePos - fragWorldPos);
    float attW = attenuation(distW);
    float NdotLw = max(dot(N, Lw), 0.0);
    vec3 whiteDiffuse = albedo * NdotLw * whiteColor * (whiteIntensity * attW);
    vec3 Hw = normalize(Lw + V);
    float NdotHw = max(dot(N, Hw), 0.0);
    float specFactorW = pow(NdotHw, shininess);
    vec3 whiteSpecular = specularStrength * specFactorW * mix(vec3(0.04), albedo, metalness) * whiteColor * (whiteIntensity * attW);

    // Red light contributions
    vec3 Lr = normalize(redPos - fragWorldPos);
    float distR = length(redPos - fragWorldPos);
    float attR = attenuation(distR);
    float NdotLr = max(dot(N, Lr), 0.0);
    vec3 redDiffuse = albedo * NdotLr * redColor * (redIntensity * attR);
    vec3 Hr = normalize(Lr + V);
    float NdotHr = max(dot(N, Hr), 0.0);
    float specFactorR = pow(NdotHr, shininess);
    vec3 redSpecular = specularStrength * specFactorR * mix(vec3(0.04), albedo, metalness) * redColor * (redIntensity * attR);

    vec3 ambient = ambientColor * albedo;
    vec3 diffuseSum = whiteDiffuse + redDiffuse;
    vec3 specularSum = whiteSpecular + redSpecular;

    vec3 resultLinear = ambient + diffuseSum + specularSum;
    vec3 resultGamma = pow(resultLinear, vec3(1.0 / GAMMA));
    outColor = vec4(resultGamma, 1.0);
}