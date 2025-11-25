#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <tuple>
#include "Mesh.h"
#include "Pipelines/Pipeline.h"
#include "Buffer.h"
#include "Texture.h"

// Forward declarations
class VulkanContext;

namespace Engine
{
    struct UniformBufferObject;
    struct PushConstantModel;

    class Object
    {
    public:
        Object() = default;
        ~Object() = default;

        // Prevent copying
        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;

        // Allow moving
        Object(Object&&) noexcept = default;
        Object& operator=(Object&&) noexcept = default;

        // Create object with mesh and pipeline
        void create(
            VulkanContext& context,
            Mesh&& mesh,
            Pipeline&& pipeline,
            VkDescriptorSetLayout descriptorSetLayout,
            VkDescriptorPool descriptorPool,
            const std::vector<Buffer>& uniformBuffers,
            Engine::Texture& albedoTex,
            Engine::Texture& normalTex
        );

        // Cleanup resources
        void cleanup(VulkanContext& context);

        // Draw the object
        void draw(
            VkCommandBuffer commandBuffer,
            uint32_t currentFrame,
            const glm::mat4& modelMatrix = glm::mat4(1.0f),
            int textureIndex = 0
        );

        // Update uniform buffer data for a specific frame
        void updateUniformBuffer(
            VkDevice device,
            uint32_t frameIndex,
            const void* data,
            size_t dataSize
        );

        // Bind descriptor set for this object
        void bindDescriptorSet(
            VkCommandBuffer commandBuffer,
            uint32_t currentFrame
        );

        // Getters
        Mesh& getMesh() { return mesh; }
        const Mesh& getMesh() const { return mesh; }
        Pipeline& getPipeline() { return pipeline; }
        const std::vector<VkDescriptorSet>& getDescriptorSets() const { return descriptorSets; }

        // Set transform
        void setTransform(const glm::mat4& transform) { modelMatrix = transform; }
        glm::mat4 getTransform() const { return modelMatrix; }

        void setPushConstantStages(VkShaderStageFlags stages) { pushConstantStages = stages; }
        void addPushconstantStage(VkShaderStageFlagBits stage) { pushConstantStages |= stage; }
        void removePushconstantStage(VkShaderStageFlagBits stage) { pushConstantStages &= ~stage; }

        // Returns descriptor image infos for albedo (binding 1) and normal (binding 2)
        std::tuple<VkDescriptorImageInfo, VkDescriptorImageInfo> getImages();
		const Texture& getAlbedoTexture() const { return albedoTexture; }
		const Texture& getNormalTexture() const { return normalTexture; }


    private:
        Mesh mesh;
        Pipeline pipeline;
        std::vector<VkDescriptorSet> descriptorSets;
        std::vector<Buffer*> uniformBuffers;
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        VkShaderStageFlags pushConstantStages = VK_SHADER_STAGE_VERTEX_BIT;
        Engine::Texture albedoTexture;
        Engine::Texture normalTexture;
    };
}