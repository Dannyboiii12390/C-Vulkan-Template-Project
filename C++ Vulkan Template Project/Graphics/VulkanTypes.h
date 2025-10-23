#pragma once
#include <glm/glm.hpp> 
#include <vulkan/vulkan_core.h>
#include <vector>
#include <optional>


namespace Engine 
{
	// binding locations must be distinct for each binding description
    // Attribute descriptions must use distinct locations and valid formats.
    // location 0 -> position (vec3)
    // location 1 -> color    (vec3)
    // location 2 -> instance offset (vec3) bound to binding 1


    struct Vertex 
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec3 normal;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    struct InstanceData {
        glm::vec3 offset;

        static VkVertexInputBindingDescription getBindingDescription();
    };

    struct PushConstantModel
    {
        glm::mat4 model;
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const;
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct UniformBufferObject {
        //alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    
}


