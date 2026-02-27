#include "Object.h"
#include "../VulkanContext.h"
#include "VulkanTypes.h"
#include <array>
#include "../Core/Flattener.h"
#include "../Core/ModelLoader.h"

namespace Engine
{
	void Object::initialiseMeshAndPipelines(const VulkanContext& context, Mesh&& meshData, Pipeline&& pPipeline, std::vector<Buffer>& uniformBuffersRef, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout)
	{
		// Move mesh data
		mesh = std::move(meshData);

		// Store pipeline reference
		pipeline = std::move(pPipeline);

		// Store references to uniform buffers
		uniformBuffers.clear();
		uniformBuffers.reserve(uniformBuffersRef.size());
		for (auto& buffer : uniformBuffersRef)
		{
			uniformBuffers.push_back(&buffer);
		}

		descriptorPoolHandle = descriptorPool;
		// Allocate descriptor sets (one per swapchain image)
		uint32_t swapchainImageCount = static_cast<uint32_t>(uniformBuffersRef.size());
		descriptorSets.resize(swapchainImageCount);

		std::vector<VkDescriptorSetLayout> layouts(swapchainImageCount, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = swapchainImageCount;
		allocInfo.pSetLayouts = layouts.data();

		const VkDevice device = context.getDevice();
		const VkResult result = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());
		if (result != VK_SUCCESS)
		{
			std::cerr << "ERROR: vkAllocateDescriptorSets failed with code " << result
				<< " while allocating " << swapchainImageCount << " descriptor sets for object '"
				<< name << "'." << std::endl;
			throw std::runtime_error("vkAllocateDescriptorSets failed: " + std::to_string(static_cast<int>(result)));
		}
		
	}

	void Object::create(
		VulkanContext& context,
		Mesh&& meshData,
		Pipeline&& pPipeline,
		VkDescriptorSetLayout descriptorSetLayout,
		VkDescriptorPool descriptorPool,
		std::vector<Buffer>& uniformBuffersRef,
		Engine::Texture& albedoTex,
		Engine::Texture& normalTex
	)
	{
		// Store textures (these are lightweight handle copies; VulkanContext remains owner)
		albedoTexture = albedoTex.clone(context.getDevice());
		normalTexture = normalTex.clone(context.getDevice());

		initialiseMeshAndPipelines(context, std::move(meshData), std::move(pPipeline), uniformBuffersRef, descriptorPool, descriptorSetLayout);

		const uint32_t swapchainImageCount = static_cast<uint32_t>(uniformBuffersRef.size());
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i]->getBuffer();
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

			std::vector<VkWriteDescriptorSet> writes;
			writes.push_back(bufferWrite);

			// Albedo texture binding (binding = 1) - only if we have a valid view
			if (albedoTexture.imageView != VK_NULL_HANDLE)
			{
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

				writes.push_back(albedoWrite);
			}

			// Normal texture binding (binding = 2)
			if (normalTexture.imageView != VK_NULL_HANDLE)
			{
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

				writes.push_back(normalWrite);
			}

			// Skybox cubemap binding (binding = 3) - use context-provided cubemap if available
			const VkImageView skyView = context.getSkyboxCubemapView();
			const VkSampler skySampler = context.getSkyboxCubemapSampler();

			if (skyView != VK_NULL_HANDLE && skySampler != VK_NULL_HANDLE)
			{
				VkDescriptorImageInfo skyboxImageInfo{};
				skyboxImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				skyboxImageInfo.imageView = skyView;
				skyboxImageInfo.sampler = skySampler;

				VkWriteDescriptorSet skyboxWrite{};
				skyboxWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				skyboxWrite.dstSet = descriptorSets[i];
				skyboxWrite.dstBinding = 3;
				skyboxWrite.dstArrayElement = 0;
				skyboxWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				skyboxWrite.descriptorCount = 1;
				skyboxWrite.pImageInfo = &skyboxImageInfo;

				writes.push_back(skyboxWrite);
			}

			// If no writes (shouldn't happen because UBO exists), skip
			if (!writes.empty())
			{
				vkUpdateDescriptorSets(context.getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
			}
			else
			{
				std::cerr << "WARNING: Object '" << name << "' has no descriptor writes for set " << i << std::endl;
			}
		}
	}
	void Object::create(
		const VulkanContext& context,
		Mesh&& meshData,
		Pipeline&& pPipeline,
		VkDescriptorSetLayout descriptorSetLayout,
		VkDescriptorPool descriptorPool,
		std::vector<Buffer>& uniformBuffersRef
	)
	{
		initialiseMeshAndPipelines(context, std::move(meshData), std::move(pPipeline), uniformBuffersRef, descriptorPool, descriptorSetLayout);

		// Update descriptor sets to point only to uniform buffers (binding 0) and to the skybox cubemap (binding 3)
		const uint32_t swapchainImageCount = static_cast<uint32_t>(uniformBuffersRef.size());
		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i]->getBuffer();
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

			std::vector<VkWriteDescriptorSet> writes;
			writes.push_back(bufferWrite);

			// Skybox cubemap binding (binding = 3) - use context-provided cubemap if available
			const VkImageView skyView = context.getSkyboxCubemapView();
			const VkSampler skySampler = context.getSkyboxCubemapSampler();
			if (skyView != VK_NULL_HANDLE && skySampler != VK_NULL_HANDLE)
			{
				VkDescriptorImageInfo skyboxImageInfo{};
				skyboxImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				skyboxImageInfo.imageView = skyView;
				skyboxImageInfo.sampler = skySampler;

				VkWriteDescriptorSet skyboxWrite{};
				skyboxWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				skyboxWrite.dstSet = descriptorSets[i];
				skyboxWrite.dstBinding = 3;
				skyboxWrite.dstArrayElement = 0;
				skyboxWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				skyboxWrite.descriptorCount = 1;
				skyboxWrite.pImageInfo = &skyboxImageInfo;

				writes.push_back(skyboxWrite);
			}

			if (!writes.empty())
			{
				vkUpdateDescriptorSets(context.getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
			}
		}
	}

	void Object::cleanup(VulkanContext& context)
	{
		// Cleanup mesh
		mesh.cleanup(context);

		const VkDevice device = context.getDevice();
		//cleanupTexture(albedoTexture, device);
		//cleanupTexture(normalTexture, device);
		albedoTexture.destroy(device);
		normalTexture.destroy(device);
		// Clear descriptor sets (they're owned by the descriptor pool)
		if (!descriptorSets.empty() && descriptorPoolHandle != VK_NULL_HANDLE) {
			const VkDevice device = context.getDevice();
			vkFreeDescriptorSets(device, descriptorPoolHandle,
				static_cast<uint32_t>(descriptorSets.size()),
				descriptorSets.data());
			// ignore/handle result as appropriate
		}
		descriptorSets.clear();
		descriptorPoolHandle = VK_NULL_HANDLE;

		// Clear uniform buffer references
		uniformBuffers.clear();

		// Reset pipeline pointer
		pipeline.destroy(context.getDevice());
	}

	void Object::draw(
		VkCommandBuffer commandBuffer,
		uint32_t currentFrame,
		const glm::mat4& modelMat)
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
	void Object::draw(VkCommandBuffer commandBuffer, uint32_t currentFrame)
	{
		draw(commandBuffer, currentFrame, getTransformMatrix());
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
	std::tuple<VkDescriptorImageInfo, VkDescriptorImageInfo> Object::getImages() const {
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
	void Object::updateCubemapBinding(const VulkanContext& context, VkImageView cubemapView, VkSampler cubemapSampler)
	{
		const uint32_t swapchainImageCount = static_cast<uint32_t>(descriptorSets.size());
		const VkDevice device = context.getDevice();

		for (size_t i = 0; i < swapchainImageCount; i++)
		{
			VkDescriptorImageInfo skyboxImageInfo{};
			skyboxImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			skyboxImageInfo.imageView = cubemapView;
			skyboxImageInfo.sampler = cubemapSampler;

			VkWriteDescriptorSet skyboxWrite{};
			skyboxWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			skyboxWrite.dstSet = descriptorSets[i];
			skyboxWrite.dstBinding = 3; // ensure this is the binding your object shader expects
			skyboxWrite.dstArrayElement = 0;
			skyboxWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			skyboxWrite.descriptorCount = 1;
			skyboxWrite.pImageInfo = &skyboxImageInfo;

			vkUpdateDescriptorSets(device, 1, &skyboxWrite, 0, nullptr);

		}
	}

}