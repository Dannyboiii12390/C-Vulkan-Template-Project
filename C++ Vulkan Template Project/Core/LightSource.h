#pragma once

#include <glm/glm.hpp>
#include "../Graphics/Object.h"

namespace Engine { struct UniformBufferObject; }

namespace Engine
{

    class LightSource final
    {
    public:
        enum class Type : int { Directional = 0, Point = 1, Spot = 2 };

        explicit LightSource(Type type = Type::Point, const glm::vec3& color = glm::vec3(1.0f), const glm::vec3& position = glm::vec3(1.0f), float intensity = 1.0f) noexcept;
        void create(VulkanContext& context, Type type, const glm::vec3& color, const glm::vec3& position, float intensity, VkDescriptorSetLayout layout, VkDescriptorPool pool, std::vector<Buffer>& buffers, std::string_view texPath = "Objects/sun_alb.jpg") noexcept;
        // Basic setters/getters
        inline void setType(Type t) noexcept { type_ = t; }
        inline Type getType() const noexcept { return type_; }
        inline void setPosition(const glm::vec3& pos) noexcept { position_ = pos; }
        inline const glm::vec3& getPosition() const noexcept { return position_; }
        inline void setColor(const glm::vec3& c) noexcept { color_ = c; }
        inline const glm::vec3& getColor() const noexcept { return color_; }
        inline void setIntensity(float v) noexcept { intensity_ = v; }
        inline float getIntensity() const noexcept { return intensity_; }

        // Orbit / animation helpers
        // Enable orbiting the light around `center` with `radius` and `speed` (radians/sec).
        void enableOrbit(const glm::vec3& center, float radius, float speed, float height = 0.0f, float angleOffset = 0.0f) noexcept;
        void disableOrbit() noexcept;
        inline bool isOrbitEnabled() const noexcept { return orbitEnabled_; }
        glm::vec3 getDirection() const noexcept;

        // Update internal animated state. `time` is seconds since start.
        // This only modifies the `position` when orbit is enabled.
        void update(float timeSeconds) noexcept;

        void cleanup(VulkanContext& context);
        void draw(VkCommandBuffer cmdBuff, uint32_t curFrame);

        // Apply the light's data to the provided UBO.
        // `slot` selects which UBO field to write to (0 -> lightPos, 1 -> redLightPos).
        // The function only writes positions expected by the existing UBO layout used in this project.
        void applyToUBO(Engine::UniformBufferObject& ubo, int slot = 0) const noexcept;

    private:
        // Group same-sized/vector members together to improve layout
        Type type_;

        // primary vector members grouped
        glm::vec3 position_;
        glm::vec3 color_;
        glm::vec3 orbitCenter_;

        // scalar members grouped
        float intensity_;
        float orbitRadius_;
        float orbitSpeed_;
        float orbitHeight_;
        float orbitAngleOffset_;
        bool orbitEnabled_;

        Object obj;
    };

} // namespace Graphics