#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

// Forward declarations
class VulkanContext;

namespace Engine
{

    struct ShaderDescriptorBinding final
    {
        uint32_t binding;
        VkDescriptorType descriptorType;
        uint32_t descriptorCount;
        VkShaderStageFlags stageFlags;
    };

    class Shader final
    {
    public:
        Shader() = default;
        ~Shader() = default;

        // Prevent copying
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        // Initialize shader with shader files and descriptor layout
        void create(
            VulkanContext& context,
            const std::string& vertShaderPath,
            const std::string& fragShaderPath,
            const std::vector<ShaderDescriptorBinding>& bindings,
            VkFormat colorFormat,
            VkFormat depthFormat,
            uint32_t swapchainImageCount,
            VkPushConstantRange* pushConstantRange = nullptr
        );

        // Create descriptor sets for this shader
        void createDescriptorSets(
            VkDevice device,
            VkDescriptorPool descriptorPool,
            uint32_t swapchainImageCount
        );

        // Update descriptor sets with resources
        void updateDescriptorSet(
            VkDevice device,
            uint32_t frameIndex,
            const std::vector<VkDescriptorBufferInfo>& bufferInfos,
            const std::vector<VkDescriptorImageInfo>& imageInfos
        );

        // Bind shader pipeline and descriptor set for rendering
        void bind(VkCommandBuffer commandBuffer, uint32_t frameIndex) const;

        // Cleanup resources
        void destroy(VkDevice device);

        // Getters
        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
        VkDescriptorSet getDescriptorSet(uint32_t index) const
        {
            return index < descriptorSets.size() ? descriptorSets[index] : VK_NULL_HANDLE;
        }
        const std::vector<VkDescriptorSet>& getDescriptorSets() const { return descriptorSets; }
        VkPipeline getPipeline() const { return pipeline; }
        VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }

    protected:
        // Pipeline creation (can be overridden for custom pipelines)
        virtual void createPipeline(
            VulkanContext& context,
            VkFormat colorFormat,
            VkFormat depthFormat,
            VkPushConstantRange* pushConstantRange
        );

        // Helper methods
        std::vector<char> readFile(const std::string& filename);
        VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);

    private:
        // Shader modules
        VkShaderModule vertShaderModule = VK_NULL_HANDLE;
        VkShaderModule fragShaderModule = VK_NULL_HANDLE;

        // Pipeline resources
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        // Descriptor resources
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptorSets;
        std::vector<ShaderDescriptorBinding> bindings;

        // Shader paths (for debugging/recreation)
        std::string vertShaderPath;
        std::string fragShaderPath;
    };

} // namespace Engine