#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <tuple>
#include "Mesh.h"
#include "Pipelines/Pipeline.h"
#include "Buffer.h"
#include "Texture.h"
#include "Transform.h"

// Forward declarations
class VulkanContext;

namespace Engine
{
    struct UniformBufferObject;
    struct PushConstantModel;

    class Object final
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
            std::vector<Buffer>& uniformBuffers,
            Engine::Texture& albedoTex,
            Engine::Texture& normalTex
        );
        void create(
            const VulkanContext& context,
            Mesh&& meshData,
            Pipeline&& pPipeline,
            VkDescriptorSetLayout descriptorSetLayout,
            VkDescriptorPool descriptorPool,
            std::vector<Buffer>& uniformBuffersRef
        );

        // Cleanup resources
        void cleanup(VulkanContext& context);

        void initialiseMeshAndPipelines(const VulkanContext& context, Mesh&& meshData, Pipeline&& pPipeline, std::vector<Buffer>& uniformBuffersRef, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
        // Draw the object
        void draw(
            VkCommandBuffer commandBuffer,
            uint32_t currentFrame,
            const glm::mat4& modelMatrix);
        void draw(VkCommandBuffer commandBuffer, uint32_t currentFrame);

        inline glm::mat4 getTransformMatrix() const { return transform.getMatrix(); }

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

        void updateCubemapBinding(const VulkanContext& context, VkImageView cubemapView, VkSampler cubemapSampler);

        // Getters
        const Mesh& getMesh() const { return mesh; }
        const Pipeline& getPipeline() const { return pipeline; }
        const std::vector<VkDescriptorSet>& getDescriptorSets() const { return descriptorSets; }


        void setPushConstantStages(VkShaderStageFlags stages) { pushConstantStages = stages; }
        void addPushconstantStage(VkShaderStageFlagBits stage) { pushConstantStages |= stage; }
        void removePushconstantStage(VkShaderStageFlagBits stage) { pushConstantStages &= ~stage; }

		void setPosition(const glm::vec3& pos) { transform.position = pos; }
		void setRotation(const glm::vec3& rot) { transform.rotation = rot; }
		void setScale(const glm::vec3& scl) { transform.scale = scl; }
		const glm::vec3& getPosition() const { return transform.position; }
		const glm::vec3& getScale() const { return transform.scale; }
		const glm::vec3& getRotation() const { return transform.rotation; }

        // Returns descriptor image infos for albedo (binding 1) and normal (binding 2)
        std::tuple<VkDescriptorImageInfo, VkDescriptorImageInfo> getImages() const;
		const Texture& getAlbedoTexture() const { return albedoTexture; }
		const Texture& getNormalTexture() const { return normalTexture; }
		const bool isActive() const { return active; }
		void setActive(bool isActive) { active = isActive; }

		std::string_view getName() const { return name; }
		void setName(const std::string_view newName) { name = newName; }
        //todo: parasoft
        //Severity 3 - Medium	[Line 95] The parameter 'newName' of function 'setName' is passed by value	Optimization	Object.h

    private:

        std::string name;

        Engine::Transform transform { glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f) };

        Mesh mesh;
        Pipeline pipeline;
        std::vector<VkDescriptorSet> descriptorSets;
        std::vector<Buffer*> uniformBuffers;
		VkDescriptorPool descriptorPoolHandle = VK_NULL_HANDLE;
        Engine::Texture albedoTexture;
        Engine::Texture normalTexture;
        VkShaderStageFlags pushConstantStages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        bool active = true;
    };


}