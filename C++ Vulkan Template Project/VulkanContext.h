#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Core/Window.h"
#include "Graphics/VulkanTypes.h"
#include "Graphics/Mesh.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Swapchain.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>

// --- Configuration ---
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


class VulkanContext {
public:
    void run();
    VulkanContext();

    // --- Vulkan Core Components ---
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;

    // --- Core Application Members ---
    Engine::Window window;
	Engine::Mesh mesh;

    // --- Swapchain ---
	Engine::Swapchain swapChain;

    // --- Graphics Pipeline ---
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	Engine::Pipeline pipeline;

	// --- Buffers ---
    std::vector<Engine::Buffer> uniformBuffers;
    Engine::Buffer instanceBuffer;
    std::vector<Engine::InstanceData> instanceData;

    // --- Descriptors ---
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    // --- Synchronization ---
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    
    void loadInstanceData();
    void createInstanceBuffer();

    // --- Main Flow ---
    void mainLoop();
    void cleanup();

    // --- Vulkan Initialization Steps ---
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createDescriptorSetLayout();
    void createCommandPool();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();

    // --- Drawing and Swapchain Handling ---
    void drawFrame();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentImage);

    // --- Helper Functions ---
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    Engine::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    Engine::SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();

	// --- Cleanup Helpers ---
	template<typename T>
    void cleanSemaphores(std::vector<T>& semaphores) 
    {
        for (T& sem : semaphores) {
            if (sem != VK_NULL_HANDLE) {
                vkDestroySemaphore(device, sem, nullptr);
                sem = VK_NULL_HANDLE;
            }
        }
        semaphores.clear();
    }
    void cleanFences(std::vector<VkFence>& fences);
};