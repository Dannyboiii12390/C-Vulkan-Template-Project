#version 450

/* PSEUDOCODE (plan)
 - Keep all uniform and input declarations intact.
 - In main(): sample the albedo texture as before.
 - Comment out the entire lighting block (normal map sampling, TBN, lighting calculations, fresnel, tinting, gamma correction).
 - After the commented block, output the albedo directly so the shader renders the texture without lighting.
 - Preserve the original lighting code inside the comment so it can be re-enabled easily.
*/

// Push constants (model only)
layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

// UBO (binding = 0)
layout(std140, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    vec3 redLightPos;
    vec3 eyePos;
    float time;
} ubo;

// Single texture samplers (no arrays, no texIndex)
layout(binding = 1) uniform sampler2D albedoTexture;
layout(binding = 2) uniform sampler2D normalTexture;

// Inputs from vertex shader (locations must match vertex shader outputs)
layout(location = 0) in vec3 vWorldPos;    // world-space position
layout(location = 1) in vec3 vNormal;      // world-space normal
layout(location = 2) in vec2 vTexCoord;    // UV
layout(location = 3) in vec3 vTangent;     // world-space tangent
layout(location = 4) in vec3 vBitangent;   // world-space bitangent

layout(location = 0) out vec4 outColor;

// Simple PBR-lite / Blinn-Phong style with normal mapping (lighting code commented out)
void main()
{
    // Sample albedo
    vec4 albedo = texture(albedoTexture, vTexCoord);

    /*
    // --- Lighting code commented out ---
    // Sample normal map and transform from [0,1] -> [-1,1]
    vec3 nSample = texture(normalTexture, vTexCoord).xyz * 2.0 - 1.0;

    // Build TBN matrix (assume tangent/bitangent/normal are orthogonal and in world space)
    vec3 T = normalize(vTangent);
    vec3 B = normalize(vBitangent);
    vec3 N = normalize(vNormal);
    mat3 TBN = mat3(T, B, N);

    // Transform normal-map normal into world space
    vec3 Nw = normalize(TBN * nSample);

    // Lighting params
    vec3 viewDir = normalize(ubo.eyePos - vWorldPos);

    // First light (white)
    vec3 L1 = normalize(ubo.lightPos - vWorldPos);
    float diff1 = max(dot(Nw, L1), 0.0);
    vec3 reflect1 = reflect(-L1, Nw);
    float spec1 = pow(max(dot(viewDir, reflect1), 0.0), 32.0);

    // Second light (red)
    vec3 L2 = normalize(ubo.redLightPos - vWorldPos);
    float diff2 = max(dot(Nw, L2), 0.0);
    vec3 reflect2 = reflect(-L2, Nw);
    float spec2 = pow(max(dot(viewDir, reflect2), 0.0), 32.0);

    // Compose lighting
    vec3 ambient = 0.08 * albedo.rgb;
    vec3 diffuse = (diff1 + diff2) * albedo.rgb;
    vec3 specular = (spec1 * vec3(1.0) + spec2 * vec3(1.0)) * 0.25; // lower spec factor

    // Optionally add a small fresnel / view dependent term (subtle)
    float fresnel = pow(1.0 - max(dot(viewDir, Nw), 0.0), 5.0) * 0.08;

    // Red light tint
    vec3 redTint = vec3(1.0, 0.4, 0.4);
    diffuse += diff2 * (albedo.rgb * 0.15) * (redTint);
    specular *= 1.0 + diff2 * 0.2 * redTint;

    vec3 color = ambient + diffuse + specular + fresnel;

    // Gamma-correct (assumes framebuffer expects sRGB)
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, albedo.a);
    // --- End commented lighting code ---
    */

    // Output albedo directly (lighting disabled)
    outColor = albedo;
}