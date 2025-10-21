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

        VkVertexInputAttributeDescription normalAttr{};
        colorAttr.location = 2;
        colorAttr.binding = 0; // same vertex buffer
        colorAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
        colorAttr.offset = static_cast<uint32_t>(offsetof(Vertex, normal));
        attributes.push_back(colorAttr);

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
