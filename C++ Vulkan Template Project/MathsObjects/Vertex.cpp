#include "Vertex.h"
#include "InstanceData.h"

namespace Engine 
{
    // --- Vertex Implementation ---
    VkVertexInputBindingDescription Vertex::getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    std::array<VkVertexInputBindingDescription, 2> Vertex::getBindingDescriptions() {
        std::array<VkVertexInputBindingDescription, 2> bindingDescriptions{};
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(InstanceData);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        return bindingDescriptions;
    }
    std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0] = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) };
        attributeDescriptions[1] = { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) };
        attributeDescriptions[2] = { 2, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, offset) };
        return attributeDescriptions;
    }
}
