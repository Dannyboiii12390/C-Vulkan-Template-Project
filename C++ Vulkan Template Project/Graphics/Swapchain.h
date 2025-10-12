#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <cstdint>

class VulkanContext;

class Swapchain {
public:
    // Swapchain handle and associated data
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D extent = { 0, 0 };
    uint32_t imageCount = 0;

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
    uint32_t acquireNextImage(VkDevice device, VkSemaphore imageAvailableSemaphore, uint64_t timeout = UINT64_MAX);

    // Present the current image
    VkResult present(VkQueue presentQueue, uint32_t imageIndex, VkSemaphore renderFinishedSemaphore);
};
