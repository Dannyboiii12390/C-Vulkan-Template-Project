#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "Mesh.h"
#include "Buffer.h"
#include "VulkanTypes.h"
#include "Pipelines/ParticlePipeline.h"
#include "Transform.h"

class VulkanContext;

namespace Engine
{
    class ParticleSystem final
    {
    public:
        ParticleSystem() = default;
        ~ParticleSystem() = default;

        // Non-copyable
        ParticleSystem(const ParticleSystem&) = delete;
        ParticleSystem& operator=(const ParticleSystem&) = delete;

        // Movable
        ParticleSystem(ParticleSystem&&) noexcept = default;
        ParticleSystem& operator=(ParticleSystem&&) noexcept = default;

        // Create the particle system (allocates descriptor sets, stores refs to uniform buffers)
        void create(
            const VulkanContext& context,
            Mesh&& meshData,
            ParticlePipeline&& pipelineData,
            VkDescriptorSetLayout descriptorSetLayout,
            VkDescriptorPool descriptorPool,
            std::vector<Buffer>& uniformBuffersRef
        );

        // Draw the particle system (binds pipeline & descriptor set and issues mesh draw)
        void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

        // Update (advance simulation state if needed). Default does nothing; can be extended.
        void update(float deltaTime) const;

        // Cleanup resources (frees descriptor sets, destroys pipeline/mesh if owned)
        void cleanup(VulkanContext& context);

        // Simple transform helpers
        void setPosition(const glm::vec3& p) { transform.position = p; }
        void setScale(const glm::vec3& s) { transform.scale = s; }
        const glm::vec3& getPosition() const { return transform.position; }
        const glm::vec3& getScale() const { return transform.scale; }

        void setActive(bool a) { active = a; }
        bool isActive() const { return active; }

        void setName(std::string_view n) { name = n; }
        std::string_view getName() const { return name; }

		glm::mat4 getTransformMatrix() const { return transform.getMatrix(); }

        // Debug / inspection accessors
        const Mesh& getMesh() const { return mesh; }
        const std::vector<VkDescriptorSet>& getDescriptorSets() const { return descriptorSets; }
        const ParticlePipeline& getPipeline() const { return pipeline; }

        void setPushConstantStages(VkShaderStageFlags stages) { pushConstantStages = stages; }
        void addPushconstantStage(VkShaderStageFlagBits stage) { pushConstantStages |= stage; }
        void removePushconstantStage(VkShaderStageFlagBits stage) { pushConstantStages &= ~stage; }

    private:
        std::string name;
		Engine::Transform transform { glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f) };

        Mesh mesh;
        ParticlePipeline pipeline;

        std::vector<VkDescriptorSet> descriptorSets;
        std::vector<Buffer*> uniformBuffers; // non-owning refs
        VkDescriptorPool descriptorPoolHandle = VK_NULL_HANDLE;
        VkShaderStageFlags pushConstantStages = VK_SHADER_STAGE_VERTEX_BIT;

        bool active = true;
    };
}