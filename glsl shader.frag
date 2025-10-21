#version 450

// Match C++ UniformBufferObject memory layout (std140)
layout(std140, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 eyePos;
    float padding1;
    // Each Light contains two vec3's; add pads to match std140 (16-byte slots)
    struct Light { vec3 position; float pad0; vec3 color; float pad1; };
    Light lights[2];
    float time;
    float padding2[3];
} ubo;

// Push constant material (must mirror Engine::PushConstantModel::Material)
struct Material {
    vec3 ambient;  float padA;
    vec3 diffuse;  float padB;
    vec3 specular; float shininess;
};

layout(push_constant) uniform PushConstants {
    mat4 model;
    Material material;
} pushConstants;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragWorldNormal;

layout(location = 0) out vec4 outColor;

vec3 calcLight(Light light, Material mat, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // Ambient
    vec3 ambient = mat.ambient * light.color;

    // Diffuse
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * mat.diffuse * light.color;

    // Specular (Phong)
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    vec3 specular = mat.specular * spec * light.color;

    return ambient + diffuse + specular;
}

void main()
{
    vec3 N = normalize(fragWorldNormal);
    vec3 V = normalize(ubo.eyePos - fragWorldPos);
    vec3 result = vec3(0.0);

    // sum contributions from both lights
    for (int i = 0; i < 2; ++i) {
        result += calcLight(ubo.lights[i], pushConstants.material, N, fragWorldPos, V);
    }

    // modulate by base color (if you want material diffuse already baked in material.diffuse,
    // you can also multiply differently; this preserves the push-constant material)
    outColor = vec4(result * fragColor, 1.0);
}