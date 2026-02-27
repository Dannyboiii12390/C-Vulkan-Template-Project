#pragma once

#include "Pipeline.h"
#include <utility>

class VulkanContext;

namespace Engine
{
    class ParticlePipeline final : public Pipeline
    {
    public:
        ParticlePipeline() = default;
        ~ParticlePipeline() final override;

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
            VkDescriptorSetLayout descriptorSetLayout,
            VkCullModeFlags cullMode = VK_CULL_MODE_NONE, // NEW: desired cull mode
            bool depthWrite = true                            // NEW: depth write enable
        ) final override;

        // Move constructor - delegates to base class
        ParticlePipeline(ParticlePipeline&& other) noexcept
            : Pipeline(std::move(other))
        {
        }

        // Move assignment - delegates to base class
        ParticlePipeline& operator=(ParticlePipeline&& other) noexcept
        {
            if (this != &other)
            {
                Pipeline::operator=(std::move(other));
            }
            return *this;
        }

        void destroy(VkDevice device) final override;

        VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState() final override;
        VkPipelineRasterizationStateCreateInfo createRasterizationState() final override;

    };
}
