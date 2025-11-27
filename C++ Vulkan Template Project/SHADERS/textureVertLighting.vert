#version 450

// Push constants (matches fragment)
layout(push_constant) uniform PushConstants {
    mat4 model;
} pc;

// UBO (binding = 0)
layout(std140, binding = 0) uniform UBO {
    mat4 view;
    mat4 proj;
    vec3 sun_pos;
    vec3 moon_pos;
    vec3 eyePos;
    float time;
} ubo;

// Vertex inputs (match Engine::Vertex layout)
// location 0: pos, 1: color (unused here), 2: normal, 3: texCoord, 4: tangent, 5: bitangent
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

// Outputs to fragment shader (must match frag locations)
layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;
layout(location = 3) out vec3 vTangent;
layout(location = 4) out vec3 vBitangent;
// per-vertex lighting term (ambient + contribution from lights) — fragment can modulate with albedo/normal-map
layout(location = 5) out vec3 vLighting;

void main()
{
    // Model-space -> world-space
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    vWorldPos = worldPos.xyz;

    // Standard MVP
    mat4 viewModel = ubo.view * pc.model;
    gl_Position = ubo.proj * viewModel * vec4(inPosition, 1.0);

    // Normal matrix (inverse-transpose of model for correct normal transform)
    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));
    vec3 N = normalize(normalMatrix * inNormal);

    // Transform tangent/bitangent by model (vectors) and orthogonalize to N
    mat3 model3 = mat3(pc.model);
    vec3 T = normalize(model3 * inTangent);
    // Gram-Schmidt orthogonalize T relative to N
    T = normalize(T - N * dot(N, T));
    // Recompute B as cross to ensure right-handed orthonormal frame
    vec3 B = normalize(cross(N, T));

    vNormal = N;
    vTangent = T;
    vBitangent = B;

    vTexCoord = inTexCoord;

    // --- Per-vertex Blinn-Phong lighting (basic) ---
    // Material parameters (tweakable)
    vec3 ambientColor = vec3(0.08); // low ambient
    vec3 sunColor = vec3(1.0, 0.95, 0.85);   // warm sun
    vec3 moonColor = vec3(0.6, 0.7, 1.0);    // cool moon
    float diffuseStrength = 1.0;
    float specularStrength = 0.5;
    float shininess = 32.0;

    // View direction
    vec3 V = normalize(ubo.eyePos - vWorldPos);

    // Sun contribution
    vec3 Ls = normalize(ubo.sun_pos - vWorldPos);
    float NdotLs = max(dot(N, Ls), 0.0);
    vec3 diffuseSun = diffuseStrength * NdotLs * sunColor;
    vec3 Hs = normalize(Ls + V);
    float specSun = pow(max(dot(N, Hs), 0.0), shininess) * specularStrength;
    vec3 specularSun = specSun * sunColor;

    // Moon contribution
    vec3 Lm = normalize(ubo.moon_pos - vWorldPos);
    float NdotLm = max(dot(N, Lm), 0.0);
    vec3 diffuseMoon = diffuseStrength * NdotLm * moonColor * 0.5; // weaker moon light
    vec3 Hm = normalize(Lm + V);
    float specMoon = pow(max(dot(N, Hm), 0.0), shininess) * specularStrength * 0.25;
    vec3 specularMoon = specMoon * moonColor;

    // Combine lighting: ambient + diffuse + specular
    vLighting = ambientColor + (diffuseSun + specularSun) + (diffuseMoon + specularMoon);

    // clamp to avoid extreme values
    vLighting = clamp(vLighting, 0.0, 4.0);
}