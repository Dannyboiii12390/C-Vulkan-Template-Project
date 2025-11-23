
#include "Object.h"
#include "../VulkanContext.h"
#include "VulkanTypes.h"
#include <array>

namespace Engine
{

	void Object::create(
		VulkanContext& context,
		Mesh&& meshData,
		Pipeline&& pPipeline,
		VkDescriptorSetLayout descriptorSetLayout,
		VkDescriptorPool descriptorPool,
		const std::vector<Buffer>& uniformBuffersRef,
		const Engine::Texture& albedoTex,
		const Engine::Texture& normalTex
	)
	{
		// Store textures (these are lightweight handle copies; VulkanContext remains owner)
		albedoTexture = albedoTex;
		normalTexture = normalTex;

		// Move mesh data
		mesh = std::move(meshData);

		// Store pipeline reference
		pipeline = std::move(pPipeline);

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

		// Update descriptor sets to point to uniform buffers, albedo texture, and normal texture.
		// Binding layout assumed:
		//  - binding 0: uniform buffer
		//  - binding 1: combined image sampler (albedo)
		//  - binding 2: combined image sampler (normal)
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			// Uniform buffer binding (binding = 0)
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i]->buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet bufferWrite{};
			bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			bufferWrite.dstSet = descriptorSets[i];
			bufferWrite.dstBinding = 0;
			bufferWrite.dstArrayElement = 0;
			bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bufferWrite.descriptorCount = 1;
			bufferWrite.pBufferInfo = &bufferInfo;

			// Albedo texture binding (binding = 1)
			VkDescriptorImageInfo albedoImageInfo{};
			albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			albedoImageInfo.imageView = albedoTexture.imageView;
			albedoImageInfo.sampler = context.getCurrentSampler(albedoTexture);

			VkWriteDescriptorSet albedoWrite{};
			albedoWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			albedoWrite.dstSet = descriptorSets[i];
			albedoWrite.dstBinding = 1;
			albedoWrite.dstArrayElement = 0;
			albedoWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			albedoWrite.descriptorCount = 1;
			albedoWrite.pImageInfo = &albedoImageInfo;

			// Normal texture binding (binding = 2)
			VkDescriptorImageInfo normalImageInfo{};
			normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			normalImageInfo.imageView = normalTexture.imageView;
			normalImageInfo.sampler = context.getCurrentSampler(normalTexture);

			VkWriteDescriptorSet normalWrite{};
			normalWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			normalWrite.dstSet = descriptorSets[i];
			normalWrite.dstBinding = 2;
			normalWrite.dstArrayElement = 0;
			normalWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			normalWrite.descriptorCount = 1;
			normalWrite.pImageInfo = &normalImageInfo;

			// Update all three bindings at once
			std::array<VkWriteDescriptorSet, 3> writes = { bufferWrite, albedoWrite, normalWrite };
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
		}
	}

	void Object::cleanup(VulkanContext& context)
	{
		// Cleanup mesh
		mesh.cleanup(context);

		// Cleanup textures - only destroy if valid
		auto cleanupTexture = [this](Engine::Texture& tex, VkDevice& device) {
			if (tex.nearestSampler != VK_NULL_HANDLE) {
				vkDestroySampler(device, tex.nearestSampler, nullptr);
				tex.nearestSampler = VK_NULL_HANDLE;
			}
			if (tex.bilinearSampler != VK_NULL_HANDLE) {
				vkDestroySampler(device, tex.bilinearSampler, nullptr);
				tex.bilinearSampler = VK_NULL_HANDLE;
			}
			if (tex.trilinearSampler != VK_NULL_HANDLE) {
				vkDestroySampler(device, tex.trilinearSampler, nullptr);
				tex.trilinearSampler = VK_NULL_HANDLE;
			}
			if (tex.anisotropicSampler != VK_NULL_HANDLE) {
				vkDestroySampler(device, tex.anisotropicSampler, nullptr);
				tex.anisotropicSampler = VK_NULL_HANDLE;
			}
			if (tex.sampler != VK_NULL_HANDLE) {
				vkDestroySampler(device, tex.sampler, nullptr);
				tex.sampler = VK_NULL_HANDLE;
			}
			if (tex.imageView != VK_NULL_HANDLE) {
				vkDestroyImageView(device, tex.imageView, nullptr);
				tex.imageView = VK_NULL_HANDLE;
			}
			if (tex.image != VK_NULL_HANDLE) {
				vkDestroyImage(device, tex.image, nullptr);
				tex.image = VK_NULL_HANDLE;
			}
			if (tex.imageMemory != VK_NULL_HANDLE) {
				vkFreeMemory(device, tex.imageMemory, nullptr);
				tex.imageMemory = VK_NULL_HANDLE;
			}
			};

		VkDevice device = context.getDevice();
		cleanupTexture(albedoTexture, device);
		cleanupTexture(normalTexture, device);

		// Clear descriptor sets (they're owned by the descriptor pool)
		descriptorSets.clear();

		// Clear uniform buffer references
		uniformBuffers.clear();

		// Reset pipeline pointer
		pipeline.destroy(context.getDevice());
	}

	void Object::draw(
		VkCommandBuffer commandBuffer,
		uint32_t currentFrame,
		const glm::mat4& modelMat,
		int textureIndex
	)
	{
		// Bind pipeline
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipeline());

		// Bind descriptor set (contains uniform buffer at binding 0, albedo at binding 1, normal at binding 2)
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline.getLayout(),
			0,
			1,
			&descriptorSets[currentFrame],
			0,
			nullptr
		);

		vkCmdPushConstants(commandBuffer, pipeline.getLayout(), pushConstantStages, 0, sizeof(modelMat), &modelMat);

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
		uniformBuffers[frameIndex]->write(device, data, dataSize);
	}

	void Object::bindDescriptorSet(
		VkCommandBuffer commandBuffer,
		uint32_t currentFrame
	)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getLayout(), 0, 1, &descriptorSets[currentFrame], 0, nullptr);
	}
	// Returns descriptor image infos for albedo (binding 1) and normal (binding 2)
	std::tuple<VkDescriptorImageInfo, VkDescriptorImageInfo> Object::getImages() {
		// Lambda to select the sampler from a texture based on the current filter mode

		// Albedo image info (binding = 1)
		VkDescriptorImageInfo albedoInfos{};
		albedoInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		albedoInfos.imageView = albedoTexture.imageView;
		albedoInfos.sampler = albedoTexture.anisotropicSampler;

		// Normal image info (binding = 2)
		VkDescriptorImageInfo normalInfos{};
		normalInfos.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normalInfos.imageView = normalTexture.imageView;
		normalInfos.sampler = normalTexture.anisotropicSampler;

		return { albedoInfos, normalInfos };
	}

}