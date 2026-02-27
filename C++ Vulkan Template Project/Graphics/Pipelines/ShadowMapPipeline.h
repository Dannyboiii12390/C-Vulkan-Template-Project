#pragma once

#include "Pipeline.h"

class VulkanContext;

namespace Engine {

    class ShadowMapPipeline : public Pipeline {
    public:
        ShadowMapPipeline() = default;
		~ShadowMapPipeline() override;

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

    private:
        // Inherit pipeline and pipelineLayout from Pipeline base class
    };

} // namespace Engine