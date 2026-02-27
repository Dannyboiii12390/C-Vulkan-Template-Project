#pragma once
#include <vulkan/vulkan_core.h>
#include <memory>
namespace Engine
{
    enum class TextureFilterMode
    {
        Nearest,
        Bilinear,
        Trilinear,
        Anisotropic
    };
    struct SharedImage
    {
        VkDevice device = VK_NULL_HANDLE;
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;

        SharedImage(VkDevice d, VkImage i, VkDeviceMemory m, VkImageView v)
            : device(d), image(i), imageMemory(m), imageView(v)
        {
        }

        ~SharedImage()
        {
            if (device == VK_NULL_HANDLE) return;
            if (imageView != VK_NULL_HANDLE)
            {
                vkDestroyImageView(device, imageView, nullptr);
                imageView = VK_NULL_HANDLE;
            }
            if (image != VK_NULL_HANDLE)
            {
                vkDestroyImage(device, image, nullptr);
                image = VK_NULL_HANDLE;
            }
            if (imageMemory != VK_NULL_HANDLE)
            {
                vkFreeMemory(device, imageMemory, nullptr);
                imageMemory = VK_NULL_HANDLE;
            }
        }

        SharedImage(const SharedImage&) = delete;
        SharedImage& operator=(const SharedImage&) = delete;
    };
    struct Texture final
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        VkSampler nearestSampler = VK_NULL_HANDLE;
        VkSampler bilinearSampler = VK_NULL_HANDLE;
        VkSampler trilinearSampler = VK_NULL_HANDLE;
        VkSampler anisotropicSampler = VK_NULL_HANDLE;

        std::shared_ptr<SharedImage> sharedImage;

        Texture() = default;

		const VkImageView getImageView() const { return imageView; }

        // Disable copy construction/assignment (raw handle ownership should not be duplicated)
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        // Allow move semantics
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;

        // Destroy resources owned by this Texture instance.
        // - If the Texture participates in shared ownership of the image/view/memory,
        //   those will be freed when the last shared owner is destroyed.
        void destroy(VkDevice device);

        // Create a safe logical copy of this Texture:
        // - Shares the underlying image/imageView/memory via shared ownership.
        // - Creates independent sampler objects for the copy.
        // - Requires the VkDevice that owns the original resources so new samplers can be created.
        Texture clone(VkDevice device);
    };
}
