#include "Object.h"
#include "../VulkanContext.h"
#include "VulkanTypes.h"
#include <array>

namespace Engine
{

    void Object::create(
        VulkanContext& context,
        Mesh&& meshData,
        Pipeline* pipelinePtr,
        VkDescriptorSetLayout descriptorSetLayout,
        VkDescriptorPool descriptorPool,
        const std::vector<Buffer>& uniformBuffersRef
    )
    {
        // Move mesh data
        mesh = std::move(meshData);

        // Store pipeline reference
        pipeline = pipelinePtr;

        // Store references to uniform buffers
        uniformBuffers.clear();
        uniformBuffers.reserve(uniformBuffersRef.size());
        for (auto& buffer : uniformBuffersRef)
        {
            uniformBuffers.push_back(const_cast<Buffer*>(&buffer));
        }

        // Allocate descriptor sets (one per swapchain image)
        uint32_t swapchainImageCount = static_cast<uint32_t>(uniformBuffersRef.size());
        descriptorSets.resize(swapchainImageCount);

        std::vector<VkDescriptorSetLayout> layouts(swapchainImageCount, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = swapchainImageCount;
        allocInfo.pSetLayouts = layouts.data();

        VkDevice device = context.getDevice();
        ASSERT(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) == VK_SUCCESS);

        // Update descriptor sets to point to uniform buffers
        for (size_t i = 0; i < swapchainImageCount; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i]->buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
    }

    void Object::cleanup(VulkanContext& context)
    {
        // Cleanup mesh
        mesh.cleanup(context);

        // Clear descriptor sets (they're owned by the descriptor pool)
        descriptorSets.clear();

        // Clear uniform buffer references
        uniformBuffers.clear();

        // Reset pipeline pointer
        pipeline = nullptr;
    }

    void Object::draw(
        VkCommandBuffer commandBuffer,
        uint32_t currentFrame,
        const glm::mat4& modelMat,
        int textureIndex
    )
    {
        if (!pipeline || descriptorSets.empty())
        {
            return; // Object not properly initialized
        }

        // Bind pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());

        // Bind descriptor set
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->getLayout(),
            0,
            1,
            &descriptorSets[currentFrame],
            0,
            nullptr
        );

        // Push constants (model matrix + texture index)
        struct
        {
            PushConstantModel pc;
            int texIndex;
        } pushData;
        pushData.pc.model = modelMat;
        pushData.texIndex = textureIndex;

        vkCmdPushConstants(
            commandBuffer,
            pipeline->getLayout(),
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(pushData),
            &pushData
        );

        // Bind mesh and draw
        mesh.bind(commandBuffer);
        mesh.draw(commandBuffer);
    }

    void Object::updateUniformBuffer(
        VkDevice device,
        uint32_t frameIndex,
        const void* data,
        size_t dataSize
    )
    {
        if (frameIndex >= uniformBuffers.size() || !uniformBuffers[frameIndex])
        {
            return;
        }

        uniformBuffers[frameIndex]->write(device, data, dataSize);
    }

    void Object::bindDescriptorSet(
        VkCommandBuffer commandBuffer,
        uint32_t currentFrame
    )
    {
        if (!pipeline || currentFrame >= descriptorSets.size())
        {
            return;
        }

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline->getLayout(),
            0,
            1,
            &descriptorSets[currentFrame],
            0,
            nullptr
        );
    }

}