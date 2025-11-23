#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <utility>

#include "../VulkanTypes.h"

class VulkanContext;

// Forward declarations
namespace Engine {

    class Pipeline {
    public:
        Pipeline() = default;
        virtual ~Pipeline();

        // Prevent copying
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        // Move operations: transfer ownership of Vulkan handles and dynamic state.
        Pipeline(Pipeline&& other) noexcept
            : pipelineLayout(other.pipelineLayout),
              graphicsPipeline(other.graphicsPipeline),
              dynamicStates(std::move(other.dynamicStates))
        {
            other.pipelineLayout = VK_NULL_HANDLE;
            other.graphicsPipeline = VK_NULL_HANDLE;
        }


        Pipeline& operator=(Pipeline&& other) noexcept
        {
            if (this != &other) {
                // Note: we do NOT destroy existing handles here because destruction requires a VkDevice.
                pipelineLayout = other.pipelineLayout;
                graphicsPipeline = other.graphicsPipeline;
                dynamicStates = std::move(other.dynamicStates);

                other.pipelineLayout = VK_NULL_HANDLE;
                other.graphicsPipeline = VK_NULL_HANDLE;
            }
            return *this;
        }

        // Pipeline creation with shader paths
        virtual void create(
            VulkanContext& context,
            const std::string& vertShaderPath,
            const std::string& fragShaderPath,
            VkFormat colorFormat,
            VkFormat depthFormat,
            VkDescriptorSetLayout descriptorSetLayout
        );

        // New: deep copy method
        // Creates a new Pipeline with its own Vulkan objects by calling `create` on the new instance.
        // The caller must provide the same creation parameters used to create the original pipeline.
        static Pipeline copy(VulkanContext& context, const Pipeline& original);

        // Cleanup resources
        virtual void destroy(VkDevice device);

        // Getters
        VkPipelineLayout getLayout() const { return pipelineLayout; }
        VkPipeline getPipeline() const { return graphicsPipeline; }


    protected:
        // Pipeline resources
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
		std::string vertShaderPath;
		std::string fragShaderPath;
        VkFormat colorFormat;
        VkFormat depthFormat;
        VkDescriptorSetLayout descriptorSetLayout;

        std::vector<VkDynamicState> dynamicStates =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        // Helper methods
        std::vector<char> readFile(const std::string& filename);
        VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);

        // Pipeline state setup methods
        virtual VkPipelineVertexInputStateCreateInfo createVertexInputState(const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, const std::vector<VkVertexInputBindingDescription>& bindingDescriptions);
        virtual VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState();
        virtual VkPipelineViewportStateCreateInfo createViewportState();
        virtual VkPipelineRasterizationStateCreateInfo createRasterizationState();
        virtual VkPipelineMultisampleStateCreateInfo createMultisampleState();
        virtual VkPipelineColorBlendStateCreateInfo createColorBlendState(const VkPipelineColorBlendAttachmentState& colorBlendAttachment);
        virtual VkPipelineDynamicStateCreateInfo createDynamicState();
    };
}