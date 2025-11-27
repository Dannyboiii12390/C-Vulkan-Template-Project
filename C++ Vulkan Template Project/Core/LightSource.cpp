#include "LightSource.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>
#include "../Graphics/VulkanTypes.h"

namespace Engine
{
    struct UniformBufferObject;

    LightSource::LightSource(Type type, const glm::vec3& color, const glm::vec3& position, float intensity) noexcept
        : type_(type),
        position_(position),
        color_(color),
        intensity_(intensity),
        orbitEnabled_(false),
        orbitCenter_(0.0f),
        orbitRadius_(0.0f),
        orbitSpeed_(0.0f),
        orbitHeight_(0.0f),
        orbitAngleOffset_(0.0f)
    {
    }
    void LightSource::create(Type type, const glm::vec3& color, const glm::vec3& position, float intensity) noexcept
    {
        type_ = type;
        color_ = color;
		position_ = position;
        intensity_ = intensity;
        position_ = glm::vec3(0.0f);
        orbitEnabled_ = false;
        orbitCenter_ = glm::vec3(0.0f);
        orbitRadius_ = 0.0f;
        orbitSpeed_ = 0.0f;
        orbitHeight_ = 0.0f;
        orbitAngleOffset_ = 0.0f;
	}

    void LightSource::setType(Type t) noexcept { type_ = t; }
    LightSource::Type LightSource::getType() const noexcept { return type_; }

    void LightSource::setPosition(const glm::vec3& pos) noexcept { position_ = pos; }
    glm::vec3 LightSource::getPosition() const noexcept { return position_; }

    void LightSource::setColor(const glm::vec3& c) noexcept { color_ = c; }
    glm::vec3 LightSource::getColor() const noexcept { return color_; }

    void LightSource::setIntensity(float v) noexcept { intensity_ = v; }
    float LightSource::getIntensity() const noexcept { return intensity_; }

    void LightSource::enableOrbit(const glm::vec3& center, float radius, float speed, float height, float angleOffset) noexcept
    {
        orbitEnabled_ = true;
        orbitCenter_ = center;
        orbitRadius_ = glm::max(0.0f, radius);
        orbitSpeed_ = speed;
        orbitHeight_ = height;
        orbitAngleOffset_ = angleOffset;
    }

    void LightSource::disableOrbit() noexcept
    {
        orbitEnabled_ = false;
    }

    bool LightSource::isOrbitEnabled() const noexcept { return orbitEnabled_; }

    void LightSource::update(float timeSeconds) noexcept
    {
        if (!orbitEnabled_) return;
        float angle = orbitSpeed_ * timeSeconds + orbitAngleOffset_;
        position_.x = orbitCenter_.x + orbitRadius_ * std::cos(angle);
        position_.y = orbitCenter_.y + orbitHeight_;
        position_.z = orbitCenter_.z + orbitRadius_ * std::sin(angle);
    }

    void LightSource::applyToUBO(Engine::UniformBufferObject& ubo, int slot) const noexcept
    {
        if (slot == 0)
        {
            ubo.sun_pos = position_;
			ubo.sun_color = color_;
			ubo.sun_intensity = intensity_;
        }
        else if (slot == 1)
        {
            ubo.moon_pos = position_;
            ubo.moon_color = color_;
            ubo.moon_intensity = intensity_;
        }
    }

}