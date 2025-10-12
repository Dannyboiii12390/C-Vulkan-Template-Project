#include "VulkanTypes.h"
#include <vector>

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
    std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindings;
        bindings.push_back(getBindingDescription());
        bindings.push_back(InstanceData::getBindingDescription());
        return bindings;
    }
    std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions()
    {
        //std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        //attributeDescriptions[0] = { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) };
        //attributeDescriptions[1] = { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) };
        //attributeDescriptions[2] = { 2, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(InstanceData, offset) };
        //return attributeDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributes{};

        VkVertexInputAttributeDescription posAttr{};
        posAttr.location = 0;
        posAttr.binding = 0; // vertex buffer binding
        posAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
        posAttr.offset = static_cast<uint32_t>(offsetof(Vertex, pos));
        attributes.push_back(posAttr);

        VkVertexInputAttributeDescription colorAttr{};
        colorAttr.location = 1;
        colorAttr.binding = 0; // same vertex buffer
        colorAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
        colorAttr.offset = static_cast<uint32_t>(offsetof(Vertex, color));
        attributes.push_back(colorAttr);

        VkVertexInputAttributeDescription instOffsetAttr{};
        instOffsetAttr.location = 2;
        instOffsetAttr.binding = 1; // instance buffer binding
        instOffsetAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
        instOffsetAttr.offset = static_cast<uint32_t>(offsetof(InstanceData, offset));
        attributes.push_back(instOffsetAttr);

        return attributes;
    }

	// -- - InstanceData Implementation ---
    VkVertexInputBindingDescription InstanceData::getBindingDescription() {
        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 1; // distinct binding for instance data
        bindingDesc.stride = sizeof(InstanceData);
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        return bindingDesc;
    }

	// --- QueueFamilyIndices Implementation ---
    bool QueueFamilyIndices::isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }

}
