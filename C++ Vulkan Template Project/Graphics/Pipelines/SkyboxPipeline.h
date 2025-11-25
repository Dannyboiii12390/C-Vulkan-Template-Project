#pragma once
#include "Pipeline.h"

class VulkanContext;

namespace Engine
{
	class SkyboxPipeline final : public Pipeline
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
		void destroy(VkDevice device) override;

	};
}

