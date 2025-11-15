
#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <fstream>

// Forward declarations
class VulkanContext;
namespace Engine {
    struct Vertex;

    class Pipeline {
    public:
        Pipeline() = default;
        virtual ~Pipeline();

        // Prevent copying
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        // Pipeline creation with shader paths
        virtual void create(
            VulkanContext& context,
            const std::string& vertShaderPath,
            const std::string& fragShaderPath,
            VkFormat colorFormat,
			VkFormat depthFormat,
            VkDescriptorSetLayout descriptorSetLayout
        );

        // Cleanup resources
        void destroy(VkDevice device);

        // Getters
        VkPipelineLayout getLayout() const { return pipelineLayout; }
        VkPipeline getPipeline() const { return graphicsPipeline; }

    protected:
        // Pipeline resources
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;

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