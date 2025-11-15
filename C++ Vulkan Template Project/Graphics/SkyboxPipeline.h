#pragma once


#include "Pipeline.h"
namespace Engine
{
	class SkyboxPipeline : public Pipeline
	{
	public:
		SkyboxPipeline() = default;
		~SkyboxPipeline() override = default;
		// Prevent copying
		SkyboxPipeline(const SkyboxPipeline&) = delete;
		SkyboxPipeline& operator=(const SkyboxPipeline&) = delete;

		// Create a dedicated skybox pipeline (same signature as base create)
		void create(
			VulkanContext& context,
			const std::string& vertShaderPath,
			const std::string& fragShaderPath,
			VkFormat colorFormat,
			VkFormat depthFormat,
			VkDescriptorSetLayout descriptorSetLayout
		) override;

		// Hide base destroy to also clean up skyboxPipeline
		void destroy(VkDevice device);

	private:
		// Separate handle for the skybox pipeline (kept distinct as requested)
		VkPipeline skyboxPipeline = VK_NULL_HANDLE;

	};
}

