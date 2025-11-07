
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

// Procedural bump function - generates spherical bumps in a grid
float proceduralHeight(vec2 uv) {
    // Grid parameters
    float gridSize = 5.0;  // 5x5 grid of bumps (adjust for more/fewer bumps)
    float bumpRadius = 0.4; // Size of each spherical bump
    
    // Map UV to grid space
    vec2 gridUV = uv * gridSize;
    vec2 gridCell = floor(gridUV);
    vec2 localUV = fract(gridUV);
    
    // Center of the current cell (in local space 0-1)
    vec2 cellCenter = vec2(0.5, 0.5);
    
    // Distance from current point to cell center
    float distToCenter = length(localUV - cellCenter);
    
    // Create spherical bump using smooth falloff
    float bump = 0.0;
    if (distToCenter < bumpRadius) {
        // Use sqrt to create a spherical/hemispherical shape
        float normalizedDist = distToCenter / bumpRadius;
        bump = sqrt(1.0 - normalizedDist * normalizedDist);
        
        // Alternatively, for smoother bumps use smoothstep:
        // bump = smoothstep(bumpRadius, 0.0, distToCenter);
    }
    
    return bump;
}

void main() {
    // Select the correct albedo texture using push constant index
    int idx = clamp(pushConstants.texIndex, 0, 1);
    vec3 albedo = texture(albedoTextures[idx], fragTexCoord).rgb;

    // ===== PROCEDURAL NORMAL GENERATION =====
    // Instead of sampling a height map, we compute height procedurally
    
    // Sample step size for computing derivatives
    float epsilon = 0.001;
    
    // Compute procedural height at current position and neighbors
    float h_center = proceduralHeight(fragTexCoord);
    float h_right = proceduralHeight(fragTexCoord + vec2(epsilon, 0.0));
    float h_up = proceduralHeight(fragTexCoord + vec2(0.0, epsilon));
    
    // Compute derivatives (gradients) of the procedural height
    float dHdx = (h_right - h_center) / epsilon;
    float dHdy = (h_up - h_center) / epsilon;
    
    // Bump strength parameter (adjust for stronger/weaker effect)
    const float bumpStrength = 1.5;
    
    // Reconstruct normal in TANGENT space from procedural height derivatives
    // The cross product of tangent vectors gives us the surface normal
    vec3 N_tangent = normalize(vec3(-dHdx * bumpStrength, -dHdy * bumpStrength, 1.0));

    // Tangent-space light & view vectors are provided by the vertex shader
    vec3 L = normalize(fragLightPos_tangent);
    vec3 V = normalize(fragViewPos_tangent);
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

    // Distance-based attenuation
    float dist = length(fragLightPos_tangent);
    float att = attenuation(dist);
    diffuse *= att;
    specular *= att;

    vec3 ambient = ambientColor * albedo;
    vec3 resultLinear = ambient + diffuse + specular;

    // Gamma correction
    vec3 resultGamma = pow(resultLinear, vec3(1.0 / GAMMA));
    outColor = vec4(resultGamma, 1.0);
}