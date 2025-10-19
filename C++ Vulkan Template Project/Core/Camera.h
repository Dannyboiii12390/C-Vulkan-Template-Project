#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../Graphics/VulkanTypes.h"


namespace Engine {

    class Camera {
    public:
        Camera(float fov = 45.0f, float aspectRatio = 16.0f / 9.0f, float nearPlane = 0.1f, float farPlane = 100.0f);
		void create(float fov, float aspectRatio, float nearPlane, float farPlane);


        // Matrix getters
        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjectionMatrix() const;
		Engine::UniformBufferObject getCameraUBO() const;

        // Camera movement controls
        void moveForward(const float distance);
        void moveRight(const float distance);
        void moveUp(const float distance);

        // Camera rotation controls
        void rotate(const float yaw, const float pitch);
        void lookAt(const glm::vec3& target);

        // Window resize handler
        void setAspectRatio(float aspectRatio);

        // Position and orientation getters/setters
        void setPosition(const glm::vec3& position);
        glm::vec3 getPosition() const;
        glm::vec3 getForward() const;
        glm::vec3 getRight() const;
        glm::vec3 getUp() const;

        // Projection settings
        void setPerspective(const float fov, const float aspectRatio, const float nearPlane, const float farPlane);
        void setOrthographic(const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane);

    private:
        void updateVectors();

        // Camera positioning
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

        // Euler angles
        float yaw = -90.0f;   // Rotation around Y-axis in degrees
        float pitch = 0.0f;   // Rotation around X-axis in degrees

        // Projection parameters
        float fieldOfView = 45.0f;
        float aspectRatio = 16.0f / 9.0f;
        float near = 0.1f;
        float far = 100.0f;
        bool isPerspective = true;

        // For orthographic projection
        float orthoLeft = -10.0f;
        float orthoRight = 10.0f;
        float orthoBottom = -10.0f;
        float orthoTop = 10.0f;
    };

    // Camera Uniform Buffer Object
    struct CameraUBO {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec3 position;
        alignas(4)  float padding;
    };

}