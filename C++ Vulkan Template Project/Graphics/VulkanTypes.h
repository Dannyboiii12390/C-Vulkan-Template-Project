#pragma once
#include <glm/glm.hpp> 
#include <vulkan/vulkan_core.h>
#include <vector>
#include <optional>
#include <memory>


namespace Engine 
{
	// binding locations must be distinct for each binding description
    // Attribute descriptions must use distinct locations and valid formats.


    struct Vertex 
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec3 normal;
		glm::vec2 texCoord;
        glm::vec3 tangent;
        glm::vec3 binormal;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    struct InstanceData {
        glm::vec3 offset;

        static VkVertexInputBindingDescription getBindingDescription();
    };
    struct ParticleVertex
    {
        glm::vec3 particlePos;

        static VkVertexInputBindingDescription getBindingDescription();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    struct PushConstantModel
    {
        glm::mat4 model;
    };
    struct Image {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
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
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec3 lightPos;
        alignas(16) glm::vec3 redLightPos;
		alignas(16) glm::vec3 eyePos;
        alignas(4) float time;
    };

    
}


