#pragma once

#include "Pipeline.h"

class VulkanContext;

namespace Engine
{
    class ParticlePipeline final : public Pipeline
    {
    public:
        ParticlePipeline() = default;
        ~ParticlePipeline() override = default;

        // Prevent copying
        ParticlePipeline(const ParticlePipeline&) = delete;
        ParticlePipeline& operator=(const ParticlePipeline&) = delete;

        // Create particle pipeline with blending
        void create(
            VulkanContext& context,
            const std::string& vertShaderPath,
            const std::string& fragShaderPath,
            VkFormat colorFormat,
            VkFormat depthFormat,
            VkDescriptorSetLayout descriptorSetLayout
        ) override;

        void destroy(VkDevice device) override;

        VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState() override;
        VkPipelineRasterizationStateCreateInfo createRasterizationState() override;

    };
}
