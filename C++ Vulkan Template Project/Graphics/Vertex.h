#pragma once
#include <glm/glm.hpp> 
#include <vulkan/vulkan_core.h>
#include <array>

namespace Engine 
{
    struct Vertex 
    {
        glm::vec3 pos;
        glm::vec3 color;
       
        static VkVertexInputBindingDescription getBindingDescription();
        static std::array<VkVertexInputBindingDescription, 2> getBindingDescriptions();
        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    };
}


