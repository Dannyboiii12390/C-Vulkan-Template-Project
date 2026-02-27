#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <cstdint>

class VulkanContext;
namespace Engine
{
    class Swapchain final
    {
        VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
        VkExtent2D extent = { 0, 0 };
        uint32_t imageCount = 0;
        VkFormat depthFormat = VK_FORMAT_UNDEFINED;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        std::vector<VkImageView> imageViews;
        VkImage depthImage = VK_NULL_HANDLE;
        VkFormat imageFormat = VK_FORMAT_UNDEFINED;
        std::vector<VkImage> images;

        VkImageView depthImageView = VK_NULL_HANDLE;

    public:

        // Details needed for swapchain creation/recreation
        struct SwapchainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        Swapchain() = default;
        Swapchain(const Swapchain&) = delete;
        Swapchain& operator=(const Swapchain&) = delete;
        Swapchain(Swapchain&& other) noexcept = default;
        Swapchain& operator=(Swapchain&& other) noexcept = default;
        ~Swapchain() = default;  // explicit cleanup required via destroy()

        VkFormat getImageFormat() const { return imageFormat; }
		VkFormat getDepthFormat() const { return depthFormat; }
		VkImageView getDepthImageView() const { return depthImageView; }
		uint32_t getImageCount() const { return imageCount; }
		VkSwapchainKHR getSwapchain() const { return swapchain; }
		const VkExtent2D& getExtent() const { return extent; }
		VkImage getDepthImage() const { return depthImage; }
		VkImage getImage(uint32_t index) const { return images[index]; }
		VkImageView getImageView(uint32_t index) const { return imageViews[index]; }

        // Create the swapchain and its image views
        void create(VulkanContext& ctx);

        // Recreate swapchain (for window resize, etc)
        void recreate(VulkanContext& ctx);

        // Clean up swapchain resources
        void destroy(VkDevice device);

        // Helper to clean up just image views
        void destroyImageViews(VkDevice device);

        // Query support details from physical device and surface
        static SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

        // Choose the optimal surface format
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        // Choose the best available present mode (prefer mailbox if available)
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        // Determine the extent of the swapchain images
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
            uint32_t windowWidth, uint32_t windowHeight);

        // Get the index of the next available image
        uint32_t acquireNextImage(VkDevice device, VkSemaphore imageAvailableSemaphore, uint64_t timeout = UINT64_MAX) const;

        // Present the current image
        VkResult present(VkQueue presentQueue, uint32_t imageIndex, VkSemaphore renderFinishedSemaphore) const;
    };
}
