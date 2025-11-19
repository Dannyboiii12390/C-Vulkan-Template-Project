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
        posAttr.binding = 0;
        posAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
        posAttr.offset = static_cast<uint32_t>(offsetof(Vertex, pos));
        attributes.push_back(posAttr);

        VkVertexInputAttributeDescription colorAttr{};
        colorAttr.location = 1;
        colorAttr.binding = 0;
        colorAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
        colorAttr.offset = static_cast<uint32_t>(offsetof(Vertex, color));
        attributes.push_back(colorAttr);

		VkVertexInputAttributeDescription normalAttr{};
		normalAttr.location = 2;
		normalAttr.binding = 0;
		normalAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
		normalAttr.offset = static_cast<uint32_t>(offsetof(Vertex, normal));
		attributes.push_back(normalAttr);

        VkVertexInputAttributeDescription texAttr{};
        texAttr.location = 3;
        texAttr.binding = 0;
        texAttr.format = VK_FORMAT_R32G32_SFLOAT;
        texAttr.offset = static_cast<uint32_t>(offsetof(Vertex, texCoord));
        attributes.push_back(texAttr);

        VkVertexInputAttributeDescription tangentAttr{};
        tangentAttr.location = 4;
        tangentAttr.binding = 0;
        tangentAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
        tangentAttr.offset = static_cast<uint32_t>(offsetof(Vertex, tangent));
        attributes.push_back(tangentAttr);

        VkVertexInputAttributeDescription binormalAttr{};
        binormalAttr.location = 5;
        binormalAttr.binding = 0;
        binormalAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
        binormalAttr.offset = static_cast<uint32_t>(offsetof(Vertex, binormal));
        attributes.push_back(binormalAttr);

        return attributes;
    }

    // --- ParticleVertex Implementation ---
    VkVertexInputBindingDescription ParticleVertex::getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(ParticleVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    std::vector<VkVertexInputAttributeDescription> ParticleVertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributes{};

        VkVertexInputAttributeDescription posAttr{};
        posAttr.location = 0;
        posAttr.binding = 0;
        posAttr.format = VK_FORMAT_R32G32B32_SFLOAT;
        posAttr.offset = static_cast<uint32_t>(offsetof(ParticleVertex, particlePos));
        attributes.push_back(posAttr);

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
