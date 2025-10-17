#version 450

layout(binding = 0) uniform UniformBufferObject {
    vec3 cameraPosition;
    vec3 cameraLookAt;
    vec3 cameraUp;
    float fov;
    float aspect;
    float zNear;
    float zFar;
};

layout(push_constant) uniform PushConstantModel {
    mat4 model;
} pushConstant;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

// Build view matrix from camera parameters
mat4 buildViewMatrix(vec3 eye, vec3 center, vec3 up) {
    vec3 f = normalize(center - eye);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);
    
    return mat4(
        vec4(s, 0.0),
        vec4(u, 0.0),
        vec4(-f, 0.0),
        vec4(-dot(s, eye), -dot(u, eye), dot(f, eye), 1.0)
    );
}

// Build perspective projection matrix
mat4 buildPerspectiveMatrix(float fovy, float aspect, float near, float far) {
    float tanHalfFovy = tan(fovy / 2.0);
    
    mat4 result = mat4(0.0);
    result[0][0] = 1.0 / (aspect * tanHalfFovy);
    result[1][1] = 1.0 / tanHalfFovy;
    result[2][2] = -(far + near) / (far - near);
    result[2][3] = -1.0;
    result[3][2] = -(2.0 * far * near) / (far - near);
    
    return result;
}

void main() {
    // Build view matrix in the shader
    mat4 view = buildViewMatrix(cameraPosition, cameraLookAt, cameraUp);
    
    // Build projection matrix in the shader
    mat4 proj = buildPerspectiveMatrix(radians(fov), aspect, zNear, zFar);
    
    // Calculate final position
    gl_Position = proj * view * pushConstant.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}