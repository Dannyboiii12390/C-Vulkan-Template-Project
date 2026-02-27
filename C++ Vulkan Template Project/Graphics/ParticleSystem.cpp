#include "ParticleSystem.h"
#include "../VulkanContext.h"
#include "VulkanTypes.h"
#include <array>

namespace Engine
{
    void ParticleSystem::create(
        const VulkanContext& context,
        Mesh&& meshData,
        ParticlePipeline&& pipelineData,
        VkDescriptorSetLayout descriptorSetLayout,
        VkDescriptorPool descriptorPool,
        std::vector<Buffer>& uniformBuffersRef)
    {
        // Move in mesh and pipeline
        mesh = std::move(meshData);
		pipeline = std::move(pipelineData);

        // Store references to uniform buffers (non-owning)
        uniformBuffers.clear();
        uniformBuffers.reserve(uniformBuffersRef.size());
        for (auto& buf : uniformBuffersRef) uniformBuffers.push_back(&buf);

        // store pool handle for later vkFreeDescriptorSets
        descriptorPoolHandle = descriptorPool;

        // Allocate per-swapchain-image descriptor sets (binding 0 = UBO)
        uint32_t count = static_cast<uint32_t>(uniformBuffersRef.size());
        descriptorSets.resize(count);

        std::vector<VkDescriptorSetLayout> layouts(count, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = count;
        allocInfo.pSetLayouts = layouts.data();

        const VkDevice device = context.getDevice();
        VkResult res = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());
        ASSERT(res == VK_SUCCESS);

        // Update each descriptor set with the corresponding uniform buffer
        for (uint32_t i = 0; i < count; ++i)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i]->getBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSets[i];
            write.dstBinding = 0;
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.descriptorCount = 1;
            write.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }
    }

    void ParticleSystem::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame)
    {
        // Debug dump
        const VkPipeline p = pipeline.getPipeline();
        const VkPipelineLayout pl = pipeline.getLayout();
        const VkDescriptorSet ds = (currentFrame < descriptorSets.size()) ? descriptorSets[currentFrame] : VK_NULL_HANDLE;

        ASSERT(p != VK_NULL_HANDLE);
        ASSERT(pl != VK_NULL_HANDLE);
        ASSERT(ds != VK_NULL_HANDLE);
        ASSERT(mesh.getVertexCount() > 0);

        // existing bind/draw code...
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pl, 0, 1, &ds, 0, nullptr);

		glm::mat4 modelMat = getTransformMatrix();
        vkCmdPushConstants(commandBuffer, pipeline.getLayout(), pushConstantStages, 0, sizeof(modelMat), &modelMat);

        mesh.bind(commandBuffer);
        mesh.draw(commandBuffer);
    }
    void ParticleSystem::update(float /*deltaTime*/) const
    {
        // Placeholder for simulation update logic (emitter, life, velocities, etc.)
        // Keep particle simulation on CPU if you don't use GPU compute/transform feedback,
        // or update instance/particle buffers and write them to GPU here.
    }

    void ParticleSystem::cleanup(VulkanContext& context)
    {
        // Cleanup mesh
        mesh.cleanup(context);

        // Free descriptor sets (owned by descriptor pool)
        if (!descriptorSets.empty() && descriptorPoolHandle != VK_NULL_HANDLE)
        {
            const VkDevice device = context.getDevice();
            vkFreeDescriptorSets(device, descriptorPoolHandle,
                static_cast<uint32_t>(descriptorSets.size()),
                descriptorSets.data());
            descriptorSets.clear();
            descriptorPoolHandle = VK_NULL_HANDLE;
        }

        // Destroy pipeline
        pipeline.destroy(context.getDevice());

        // Clear uniform buffer refs
        uniformBuffers.clear();
    }
}