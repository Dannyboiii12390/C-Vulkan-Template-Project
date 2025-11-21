#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS

#include <set>
#include <chrono>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Window.h"
#include "Core/InputHandler.h"

#include "Graphics/VulkanTypes.h"
#include "Graphics/Mesh.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Swapchain.h"
#include "Core/Camera.h"
#include "Core/ModelLoader.h"
#include "Graphics/SkyboxPipeline.h"
#include "Graphics/ParticlePipeline.h"

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
    VulkanContext();

    void drawFrame();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentImage);
    void cleanup();

    // --- Getters ---
	VkDevice getDevice() const { return device; }
	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
	VkCommandPool getCommandPool() const { return commandPool; }
	VkQueue getGraphicsQueue() const { return graphicsQueue; }
	VkSurfaceKHR getSurface() const { return surface; }
	Engine::Window& getWindow() { return window; }
	Engine::InputHandler& getInputHandler() { return inputHandler; }

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

    static glm::mat4 rotateAboutPoint(const glm::vec3& pivot, const float angle, const glm::vec3& axis);

    void switchTexture(int textureIndex);
	void cycleFilterMode();
    VkSampler getCurrentSampler(const Engine::Texture& texture);
    void updateDescriptorSetsForTexture(int textureIndex);
    void createDescriptorSetsForTexture(const Engine::Texture& tex, std::vector<VkDescriptorSet>& descSets);

private:

    // --- Vulkan Initialization Steps ---
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createDescriptorSetLayout();
	void createSkyboxDescriptorSetLayout();
    void createSkyboxDescriptorSets(VkImageView cubemapView, VkSampler cubemapSampler);
    void createCommandPool();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSyncObjects();

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
    void handleInput();

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
    Engine::InputHandler inputHandler;
    Engine::Mesh mesh;
    Engine::Mesh leftMesh;
	Engine::Mesh rightMesh;
    //Engine::Mesh terrain;

    Engine::Texture texture;
    Engine::Texture tileTexture;
    Engine::Texture textureNormal;
	Engine::Texture tileTextureNormal;

	Engine::TextureFilterMode currentFilterMode = Engine::TextureFilterMode::Anisotropic;
	int currentTextureIndex = 0;

    // --- Swapchain ---
    Engine::Swapchain swapChain;

    // --- Graphics Pipeline ---
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    Engine::Pipeline pipeline;
	Engine::SkyboxPipeline skyboxPipeline;
	VkDescriptorSetLayout skyboxDescriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> skyboxDescriptorSets;
	VkImageView skyboxCubemapView = VK_NULL_HANDLE;
	VkSampler skyboxCubemapSampler = VK_NULL_HANDLE;
	Engine::Mesh skyboxMesh;


    //Engine::Pipeline pipeline2;

    Engine::ParticlePipeline particlePipeline;
    Engine::Mesh particleMesh;
    VkDescriptorSetLayout particleDescriptorSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> particleDescriptorSets;

	VkImage skyboxCubeImage = VK_NULL_HANDLE;
	VkDeviceMemory skyboxCubemapMemory = VK_NULL_HANDLE;

    // --- Camera ---
    Engine::Camera camera;

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


};