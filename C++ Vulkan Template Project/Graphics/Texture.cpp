

#include "Texture.h"
#include "../Core/Debug Utils.h"
namespace Engine
{
    VkSampler createSamplerDefault(VkDevice device,
        VkFilter magFilter,
        VkFilter minFilter,
        VkSamplerMipmapMode mipmapMode,
        bool enableAnisotropy)
    {
        ASSERT(device != VK_NULL_HANDLE, "createSamplerDefault requires a valid VkDevice");

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
        const VkResult res = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
        if (res != VK_SUCCESS)
        {
            return VK_NULL_HANDLE;
        }
        return sampler;
    }


    Texture::Texture(Texture&& other) noexcept
    {
        *this = std::move(other);
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this != &other)
        {
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
        if (sharedImage)
        {
            // clear our local mirrors
            image = VK_NULL_HANDLE;
            imageView = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
            sharedImage.reset();
        }
        else
        {
            // no shared ownership -> free raw image resources (legacy behavior)
            if (imageView != VK_NULL_HANDLE) { vkDestroyImageView(device, imageView, nullptr); imageView = VK_NULL_HANDLE; }
            if (image != VK_NULL_HANDLE) { vkDestroyImage(device, image, nullptr); image = VK_NULL_HANDLE; }
            if (imageMemory != VK_NULL_HANDLE) { vkFreeMemory(device, imageMemory, nullptr); imageMemory = VK_NULL_HANDLE; }
        }
    }

    Texture Texture::clone(VkDevice device)
    {
        ASSERT(device != VK_NULL_HANDLE, "clone requires a valid VkDevice");

        Texture copyTexture;

        // Ensure sharedImage exists for the source so both share the image/view/memory safely.
        if (!sharedImage)
        {
            // move ownership of raw image resources into a new SharedImage
            // The SharedImage destructor will free these when last reference is gone.
            sharedImage = std::make_shared<SharedImage>(device, image, imageMemory, imageView);

            // IMPORTANT: transfer ownership into sharedImage by clearing the source raw handles.
            // This prevents accidental double-free or leaks if the source is destroyed or moved later.
            image = VK_NULL_HANDLE;
            imageMemory = VK_NULL_HANDLE;
            imageView = VK_NULL_HANDLE;
        }

        // Share the SharedImage with the clone
        copyTexture.sharedImage = sharedImage;

        // Mirror the raw handles on the clone so existing code can still read them.
        copyTexture.image = copyTexture.sharedImage->image;
        copyTexture.imageMemory = copyTexture.sharedImage->imageMemory;
        copyTexture.imageView = copyTexture.sharedImage->imageView;

        // Create independent sampler objects for the clone
        copyTexture.nearestSampler = createSamplerDefault(device, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, false);
        copyTexture.bilinearSampler = createSamplerDefault(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, false);
        copyTexture.trilinearSampler = createSamplerDefault(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, false);
        copyTexture.anisotropicSampler = createSamplerDefault(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, true);

        // Provide a reasonable combined sampler as well
        copyTexture.sampler = createSamplerDefault(device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, true);

        return copyTexture;
    }
}
