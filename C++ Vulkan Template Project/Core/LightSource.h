#pragma once

#include <glm/glm.hpp>

namespace Engine { struct UniformBufferObject; }

namespace Engine
{

    class LightSource
    {
    public:
        enum class Type : int { Directional = 0, Point = 1, Spot = 2 };

        LightSource(Type type = Type::Point, const glm::vec3& color = glm::vec3(1.0f), const glm::vec3& position = glm::vec3(1.0f), float intensity = 1.0f) noexcept;
        void create(Type type, const glm::vec3& color, const glm::vec3& position, float intensity = 1.0f) noexcept;

        // Basic setters/getters

        void setType(Type t) noexcept;
        Type getType() const noexcept;
        void setPosition(const glm::vec3& pos) noexcept;
        glm::vec3 getPosition() const noexcept;
        void setColor(const glm::vec3& c) noexcept;
        glm::vec3 getColor() const noexcept;
        void setIntensity(float v) noexcept;
        float getIntensity() const noexcept;

        // Orbit / animation helpers
        // Enable orbiting the light around `center` with `radius` and `speed` (radians/sec).
        void enableOrbit(const glm::vec3& center, float radius, float speed, float height = 0.0f, float angleOffset = 0.0f) noexcept;
        void disableOrbit() noexcept;
        bool isOrbitEnabled() const noexcept;

        // Update internal animated state. `time` is seconds since start.
        // This only modifies the `position` when orbit is enabled.
        void update(float timeSeconds) noexcept;

        // Apply the light's data to the provided UBO.
        // `slot` selects which UBO field to write to (0 -> lightPos, 1 -> redLightPos).
        // The function only writes positions expected by the existing UBO layout used in this project.
        void applyToUBO(Engine::UniformBufferObject& ubo, int slot = 0) const noexcept;

    private:
        Type type_;
        glm::vec3 position_;
        glm::vec3 color_;
        float intensity_;

        // orbit state
        bool orbitEnabled_;
        glm::vec3 orbitCenter_;
        float orbitRadius_;
        float orbitSpeed_;
        float orbitHeight_;
        float orbitAngleOffset_;
    };

} // namespace Graphics