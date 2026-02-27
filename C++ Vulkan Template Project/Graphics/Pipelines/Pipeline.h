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
        // Explicit default constructor that initializes all members to safe defaults
        Pipeline() noexcept
            : pipelineLayout(VK_NULL_HANDLE),
            graphicsPipeline(VK_NULL_HANDLE),
            descriptorSetLayout(VK_NULL_HANDLE),
            dynamicStates{
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            },
            fragShaderPath(),
            vertShaderPath(),
            colorFormat(VK_FORMAT_UNDEFINED),
            depthFormat(VK_FORMAT_UNDEFINED),
            cullMode(VK_CULL_MODE_BACK_BIT),
            depthWrite(true)
        {
        }
        virtual ~Pipeline();

        // Prevent copying
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        // Move operations: transfer ownership of Vulkan handles and dynamic state.
        Pipeline(Pipeline&& other) noexcept
            : pipelineLayout(other.pipelineLayout),
            graphicsPipeline(other.graphicsPipeline),
            descriptorSetLayout(other.descriptorSetLayout),
            dynamicStates(std::move(other.dynamicStates)),
            fragShaderPath(std::move(other.fragShaderPath)),
            vertShaderPath(std::move(other.vertShaderPath)),
            colorFormat(other.colorFormat),
            depthFormat(other.depthFormat),
            cullMode(other.cullMode),
            depthWrite(other.depthWrite)
        {
            other.pipelineLayout = VK_NULL_HANDLE;
            other.graphicsPipeline = VK_NULL_HANDLE;
            other.descriptorSetLayout = VK_NULL_HANDLE;
            other.colorFormat = VK_FORMAT_UNDEFINED;
            other.depthFormat = VK_FORMAT_UNDEFINED;
            other.cullMode = VK_CULL_MODE_NONE;
            other.depthWrite = false;
        }


        Pipeline& operator=(Pipeline&& other) noexcept
        {
            if (this != &other)
            {
                // Note: we do NOT destroy existing handles here because destruction requires a VkDevice.
                pipelineLayout = other.pipelineLayout;
                graphicsPipeline = other.graphicsPipeline;
                descriptorSetLayout = other.descriptorSetLayout;

                dynamicStates = std::move(other.dynamicStates);
                fragShaderPath = std::move(other.fragShaderPath);
                vertShaderPath = std::move(other.vertShaderPath);

                colorFormat = other.colorFormat;
                depthFormat = other.depthFormat;
                cullMode = other.cullMode;
                depthWrite = other.depthWrite;

                other.pipelineLayout = VK_NULL_HANDLE;
                other.graphicsPipeline = VK_NULL_HANDLE;
                other.descriptorSetLayout = VK_NULL_HANDLE;
                other.colorFormat = VK_FORMAT_UNDEFINED;
                other.depthFormat = VK_FORMAT_UNDEFINED;
                other.cullMode = VK_CULL_MODE_NONE;
                other.depthWrite = false;
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
            VkDescriptorSetLayout descriptorSetLayout,
            VkCullModeFlags cullMode = VK_CULL_MODE_NONE, // NEW: desired cull mode
            bool depthWrite = true                            // NEW: depth write enable
        );

        // New: deep copy method
        // Creates a new Pipeline with its own Vulkan objects by calling `create` on the new instance.
        // The caller must provide the same creation parameters used to create the original pipeline.
        static Pipeline copy(VulkanContext& context, const Pipeline& original);

        // Cleanup resources
        virtual void destroy(VkDevice device);

        // Getters
        VkPipelineLayout getLayout() const { return pipelineLayout; }
        void setLayout(VkPipelineLayout layout) { pipelineLayout = layout; }
        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
        void setDescriptorSetLayout(VkDescriptorSetLayout layout) { descriptorSetLayout = layout; }
        VkPipeline getPipeline() const { return graphicsPipeline; }
        void setPipeline(VkPipeline pipeline) { graphicsPipeline = pipeline; }

        const char* getName() const { return "main"; };

        // Access stored creation options
        VkCullModeFlags getCullMode() const { return cullMode; }
        bool getDepthWrite() const { return depthWrite; }

    protected:
        // Helper methods
        std::vector<char> readFile(const std::string& filename) const;
        VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) const;

        // Pipeline state setup methods
        virtual VkPipelineVertexInputStateCreateInfo createVertexInputState(const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions, const std::vector<VkVertexInputBindingDescription>& bindingDescriptions);
        virtual VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState();
        virtual VkPipelineViewportStateCreateInfo createViewportState();
        virtual VkPipelineRasterizationStateCreateInfo createRasterizationState();
        virtual VkPipelineMultisampleStateCreateInfo createMultisampleState();
        virtual VkPipelineColorBlendStateCreateInfo createColorBlendState(const VkPipelineColorBlendAttachmentState& colorBlendAttachment);
        virtual VkPipelineDynamicStateCreateInfo createDynamicState();

    private:
        // Pipeline resources
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

        std::vector<VkDynamicState> dynamicStates =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        std::string fragShaderPath;
        std::string vertShaderPath;

        VkFormat colorFormat;
        VkFormat depthFormat;

        // Creation options to preserve behavior
        VkCullModeFlags cullMode;
        bool depthWrite;
    };
}