#include "LightSource.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>
#include "../Graphics/VulkanTypes.h"
#include "../Core/ModelLoader.h"

namespace Engine
{
    struct UniformBufferObject;

    LightSource::LightSource(Type type, const glm::vec3& color, const glm::vec3& position, float intensity) noexcept
        : type_(type),
        position_(position),
        color_(color),
        orbitCenter_(0.0f),
        intensity_(intensity),
        orbitRadius_(0.0f),
        orbitSpeed_(0.0f),
        orbitHeight_(0.0f),
        orbitAngleOffset_(0.0f),
        orbitEnabled_(false)
    {
        
    }
    void LightSource::create(VulkanContext& context, Type type, const glm::vec3& color, const glm::vec3& position, float intensity, VkDescriptorSetLayout layout, VkDescriptorPool pool, std::vector<Buffer>& buffers, std::string_view texPath) noexcept
    {
        type_ = type;
        color_ = color;
		position_ = position;
        intensity_ = intensity;
        //position_ = glm::vec3(0.0f);
        orbitEnabled_ = false;
        orbitCenter_ = glm::vec3(0.0f);
        orbitRadius_ = 0.0f;
        orbitSpeed_ = 0.0f;
        orbitHeight_ = 0.0f;
        orbitAngleOffset_ = 0.0f;


		Mesh mesh = ModelLoader::createSphere(context, 1.0f, 16, 16);
        Pipeline pipeline;
        pipeline.create(
            context,
			"shaders/LightSource.vert.spv",
		    "shaders/LightSource.frag.spv",
            context.getSwapchain().getImageFormat(),
            context.getSwapchain().getDepthFormat(),
            layout
		);
		Texture sun_albedo = ModelLoader::createTextureImage(context, texPath.data(), true);
        uint8_t normalPixel[4] = { 128, 128, 255, 255 }; // normal (R=0.5, G=0.5, B=1.0), NOT sRGB
        Engine::Texture normalTex = Engine::ModelLoader::createTextureImageFromMemory(context, normalPixel, 1, 1, false);
        obj.create(context, std::move(mesh), std::move(pipeline), layout, pool, buffers, sun_albedo, normalTex);
        obj.setScale(glm::vec3(10.0f));
        sun_albedo.destroy(context.getDevice());
        normalTex.destroy(context.getDevice());
        
	}

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
    glm::vec3 LightSource::getDirection() const noexcept
    {
        // Direction from light position to orbit center
        const glm::vec3 dir = glm::normalize(orbitCenter_ - position_);
        return dir;
	}
    void LightSource::update(float timeSeconds) noexcept
    {
        if (!orbitEnabled_) return;

        // Compute angle around orbit
        const float angle = orbitSpeed_ * timeSeconds + orbitAngleOffset_;

        // Build a circle in the Y-Z plane (so the light moves around Y and Z)
        const float localY = orbitRadius_ * std::cos(angle);
        const float localZ = orbitRadius_ * std::sin(angle);

        // Interpret orbitHeight_ as a tilt angle (radians) around the X axis.
        // Rotate the (0, localY, localZ) point about the X axis by 'tilt' so
        // the orbital plane can be tilted (e.g. Earth's axial tilt).
        const float tilt = orbitHeight_;
        const float cosT = std::cos(tilt);
        const float sinT = std::sin(tilt);

        const float rotatedY = localY * cosT - localZ * sinT;
        const float rotatedZ = localY * sinT + localZ * cosT;

        // Position in world space
        position_.x = orbitCenter_.x; // keep X tied to orbit center (orbit occurs in Y-Z)
        position_.y = orbitCenter_.y + rotatedY;
        position_.z = orbitCenter_.z + rotatedZ;
    }
    void LightSource::cleanup(VulkanContext& context) {
        obj.cleanup(context);
    }
    void LightSource::draw(VkCommandBuffer cmdBuff, uint32_t curFrame) {
        obj.setPosition(position_);
        obj.draw(cmdBuff, curFrame);
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