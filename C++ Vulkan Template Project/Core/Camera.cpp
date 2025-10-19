#include "Camera.h"
#include <algorithm>

namespace Engine {

    Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
        : fieldOfView(fov), aspectRatio(aspectRatio), near(nearPlane), far(farPlane) {
        updateVectors();
    }
    void Camera::create(float fov, float aspectRatio, float nearPlane, float farPlane)
    {
		fieldOfView = fov;
		aspectRatio = aspectRatio;
		near = nearPlane;
		far = farPlane;
		updateVectors();

    }

    glm::mat4 Camera::getViewMatrix() const {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 Camera::getProjectionMatrix() const {
        if (isPerspective) {
            return glm::perspective(glm::radians(fieldOfView), aspectRatio, near, far);
        }
        else {
            return glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, near, far);
        }
    }
    Engine::UniformBufferObject Camera::getCameraUBO() const {
        Engine::UniformBufferObject ubo{};
        ubo.view = getViewMatrix();
        ubo.proj = getProjectionMatrix();
        ubo.proj[1][1] *= -1;  // Flip Y for Vulkan
        return ubo;
	}

    void Camera::moveForward(const float distance) {
        position += front * distance;
    }

    void Camera::moveRight(const float distance) {
        position += right * distance;
    }

    void Camera::moveUp(const float distance) {
        position += up * distance;
    }

    void Camera::rotate(const float yawOffset, const float pitchOffset) {
        yaw += yawOffset;
        pitch += pitchOffset;

        // Constrain pitch to avoid gimbal lock
        pitch = std::clamp(pitch, -89.0f, 89.0f);

        updateVectors();
    }

    void Camera::lookAt(const glm::vec3& target) {
        front = glm::normalize(target - position);

        // Extract yaw and pitch from front vector
        pitch = glm::degrees(asin(front.y));
        yaw = glm::degrees(atan2(front.z, front.x));

        updateVectors();
    }

    void Camera::setAspectRatio(const float ratio) {
        aspectRatio = ratio;
    }

    void Camera::setPosition(const glm::vec3& pos) {
        position = pos;
    }

    glm::vec3 Camera::getPosition() const {
        return position;
    }

    glm::vec3 Camera::getForward() const {
        return front;
    }

    glm::vec3 Camera::getRight() const {
        return right;
    }

    glm::vec3 Camera::getUp() const {
        return up;
    }

    void Camera::setPerspective(const float fov, const float ratio, const float nearPlane, const float farPlane) {
        fieldOfView = fov;
        aspectRatio = ratio;
        near = nearPlane;
        far = farPlane;
        isPerspective = true;
    }

    void Camera::setOrthographic(const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane) {
        orthoLeft = left;
        orthoRight = right;
        orthoBottom = bottom;
        orthoTop = top;
        near = nearPlane;
        far = farPlane;
        isPerspective = false;
    }

    void Camera::updateVectors() {
        // Calculate front vector from yaw and pitch
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);

        // Recalculate right and up vectors
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }

}