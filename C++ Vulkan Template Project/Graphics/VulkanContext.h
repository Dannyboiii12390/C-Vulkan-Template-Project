#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "../Core/Window.h"

class VulkanContext {
public:
    VulkanContext(Window& window);
    ~VulkanContext();

    void init();
    void cleanup();

    // Accessors for other subsystems
    VkInstance getInstance() const { return instance; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkDevice getDevice() const { return device; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkQueue getPresentQueue() const { return presentQueue; }
    VkSurfaceKHR getSurface() const { return surface; }

private:
    Window& window;

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    // Initialization steps
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();

    // Helpers
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();

#ifdef _DEBUG
    void setupDebugMessenger();
    VkDebugUtilsMessengerEXT debugMessenger;
    bool enableValidationLayers = true;
#else
    bool enableValidationLayers = false;
#endif
};


