#pragma once
#include "Pipeline.h"

class VulkanContext;

namespace Engine
{
	class SkyboxPipeline final : public Pipeline
	{
	public:
		SkyboxPipeline() = default;
		~SkyboxPipeline() final override;
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
			VkDescriptorSetLayout descriptorSetLayout,
			VkCullModeFlags cullMode = VK_CULL_MODE_NONE, // NEW: desired cull mode
			bool depthWrite = true                            // NEW: depth write enable
		) final override;

		// Hide base destroy to also clean up skyboxPipeline
		void destroy(VkDevice device) final override;

	};
}

