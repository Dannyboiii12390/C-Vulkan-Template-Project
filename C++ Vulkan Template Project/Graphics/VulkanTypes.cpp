#include "VulkanTypes.h"
#include <vector>

namespace Engine 
{
    VkSampler createSamplerDefault(VkDevice device,
        VkFilter magFilter,
        VkFilter minFilter,
        VkSamplerMipmapMode mipmapMode,
        bool enableAnisotropy)
    {
        assert(device != VK_NULL_HANDLE && "createSamplerDefault requires a valid VkDevice");

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext = nullptr;
        samplerInfo.flags = 0;
        samplerInfo.magFilter = magFilter;
        samplerInfo.minFilter = minFilter;
        samplerInfo.mipmapMode = mipmapMode;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.anisotropyEnable = enableAnisotropy ? VK_TRUE : VK_FALSE;
        samplerInfo.maxAnisotropy = enableAnisotropy ? 16.0f : 1.0f;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = std::numeric_limits<float>::max();
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        VkSampler sampler = VK_NULL_HANDLE;
        VkResult res = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
        if (res != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }
        return sampler;
    }

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

    // --- Texture Implementation ---

    // SharedImage holds the image/view/memory and destroys them when last owner goes away.
    struct Texture::SharedImage
    {
        VkDevice device = VK_NULL_HANDLE;
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;

        SharedImage(VkDevice d, VkImage i, VkDeviceMemory m, VkImageView v)
            : device(d), image(i), imageMemory(m), imageView(v) {}

        ~SharedImage()
        {
            if (device == VK_NULL_HANDLE) return;
            if (imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(device, imageView, nullptr);
                imageView = VK_NULL_HANDLE;
            }
            if (image != VK_NULL_HANDLE) {
                vkDestroyImage(device, image, nullptr);
                image = VK_NULL_HANDLE;
            }
            if (imageMemory != VK_NULL_HANDLE) {
                vkFreeMemory(device, imageMemory, nullptr);
                imageMemory = VK_NULL_HANDLE;
            }
        }

        SharedImage(const SharedImage&) = delete;
        SharedImage& operator=(const SharedImage&) = delete;
    };

    Texture::Texture(Texture&& other) noexcept
    {
        *this = std::move(other);
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this != &other) {
            // move simple handles and samplers
            image = other.image;
            imageMemory = other.imageMemory;
            imageView = other.imageView;
            sampler = other.sampler;
            nearestSampler = other.nearestSampler;
            bilinearSampler = other.bilinearSampler;
            trilinearSampler = other.trilinearSampler;
            anisotropicSampler = other.anisotropicSampler;

            // move shared ownership
            sharedImage = std::move(other.sharedImage);

            // null out source so it won't destroy in its destructor/cleanup
            other.image = VK_NULL_HANDLE;
            other.imageMemory = VK_NULL_HANDLE;
            other.imageView = VK_NULL_HANDLE;
            other.sampler = VK_NULL_HANDLE;
            other.nearestSampler = VK_NULL_HANDLE;
            other.bilinearSampler = VK_NULL_HANDLE;
            other.trilinearSampler = VK_NULL_HANDLE;
            other.anisotropicSampler = VK_NULL_HANDLE;
            other.sharedImage.reset();
        }
        return *this;
    }

    void Texture::destroy(VkDevice device)
    {
        if (device == VK_NULL_HANDLE) return;

        // destroy sampler objects owned by this Texture
        if (nearestSampler != VK_NULL_HANDLE) { vkDestroySampler(device, nearestSampler, nullptr); nearestSampler = VK_NULL_HANDLE; }
        if (bilinearSampler != VK_NULL_HANDLE) { vkDestroySampler(device, bilinearSampler, nullptr); bilinearSampler = VK_NULL_HANDLE; }
        if (trilinearSampler != VK_NULL_HANDLE) { vkDestroySampler(device, trilinearSampler, nullptr); trilinearSampler = VK_NULL_HANDLE; }
        if (anisotropicSampler != VK_NULL_HANDLE) { vkDestroySampler(device, anisotropicSampler, nullptr); anisotropicSampler = VK_NULL_HANDLE; }
        if (sampler != VK_NULL_HANDLE) { vkDestroySampler(device, sampler, nullptr); sampler = VK_NULL_HANDLE; }

        // If we participate in shared ownership of the image resources, reset shared pointer
        // and let the last owner destroy the image/view/memory in SharedImage::~SharedImage.
        if (sharedImage) {
            // clear our local mirrors
            image = VK_NULL_HANDLE;
            imageView = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
            sharedImage.reset();
        } else {
            // no shared ownership -> free raw image resources (legacy behavior)
            if (imageView != VK_NULL_HANDLE) { vkDestroyImageView(device, imageView, nullptr); imageView = VK_NULL_HANDLE; }
            if (image != VK_NULL_HANDLE) { vkDestroyImage(device, image, nullptr); image = VK_NULL_HANDLE; }
            if (imageMemory != VK_NULL_HANDLE) { vkFreeMemory(device, imageMemory, nullptr); imageMemory = VK_NULL_HANDLE; }
        }
    }

    Texture Texture::clone(VkDevice device)
    {
        assert(device != VK_NULL_HANDLE && "clone requires a valid VkDevice");

        Texture copyTexture;

        // Ensure sharedImage exists for the source so both share the image/view/memory safely.
        if (!sharedImage) {
            // move ownership of raw image resources into a new SharedImage
            // The SharedImage destructor will free these when last reference is gone.
            sharedImage = std::make_shared<SharedImage>(device, image, imageMemory, imageView);
        }

        // Share the SharedImage with the clone
        copyTexture.sharedImage = sharedImage;

        // Mirror the raw handles so existing code accessing image/imageView still works
        copyTexture.image = image;
        copyTexture.imageMemory = imageMemory;
        copyTexture.imageView = imageView;

        // Create independent sampler objects for the clone
        copyTexture.nearestSampler = createSamplerDefault(device, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, false);
        copyTexture.bilinearSampler = createSamplerDefault(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, false);
        copyTexture.trilinearSampler = createSamplerDefault(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, false);
        copyTexture.anisotropicSampler = createSamplerDefault(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, true);

        // Provide a reasonable combined sampler as well
        copyTexture.sampler = createSamplerDefault(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, true);

        return copyTexture;
    }


} // namespace Engine

