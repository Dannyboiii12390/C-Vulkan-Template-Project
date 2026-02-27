#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Core/Window.h"
#include "Core/InputHandler.h"

#include "Graphics/VulkanTypes.h"
#include "Graphics/Mesh.h"
#include "Graphics/Pipelines/Pipeline.h"
#include "Graphics/Swapchain.h"
#include "Core/Camera.h"
#include "Graphics/Pipelines/SkyboxPipeline.h"
#include "Graphics/Pipelines/ParticlePipeline.h"
#include "Graphics/Object.h"
#include "Graphics/Texture.h"
#include "Core/LightSource.h"
#include <chrono>
#include "Graphics/ParticleSystem.h"
#include "Core/ParticleManager.h"
#include "Core/GUI.h"
#include "Graphics/Pipelines/ShadowMapPipeline.h"

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


class VulkanContext final{
public:
    VulkanContext();

    //stop copying
	VulkanContext(const VulkanContext&) = delete;
	VulkanContext& operator=(const VulkanContext&) = delete;

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
	const Engine::Window& getWindow() const { return window; }
	const Engine::InputHandler& getInputHandler() const { return inputHandler; }
	const Engine::Swapchain& getSwapchain() const { return swapChain; }
	void updateInputHandler() { inputHandler.update(); }

	VkImageView getSkyboxCubemapView() const { return skyboxCubemapView; }
	VkSampler getSkyboxCubemapSampler() const { return skyboxCubemapSampler; }

    // --- Helper Functions ---
    bool isDeviceSuitable(VkPhysicalDevice device) const;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    Engine::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    Engine::SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
    std::vector<const char*> getRequiredExtensions() const;
    bool checkValidationLayerSupport() const;

	VkDescriptorSetLayout getParticleDescriptorSetLayout() const { return particleDescriptorSetLayout; }

	const Engine::Camera& getC1() const { return C1; }
	const Engine::Camera& getC2() const { return C2; }
	const Engine::Camera& getC3() const { return C3; }
	const Engine::Camera* getCamera() const { return camera; }

    VkImageView getCubemapView() const { return skyboxCubemapView; }
    VkSampler getCubemapSampler() const { return skyboxCubemapSampler; }


    static glm::mat4 rotateAboutPoint(const glm::vec3& pivot, const float angle, const glm::vec3& axis);

    VkSampler getCurrentSampler(const Engine::Texture& texture) const;
    void updateDescriptorSetsForTexture();
    void createDescriptorSetsForTexture(const Engine::Texture& tex, std::vector<VkDescriptorSet>& descSets);

	const float getDeltaTime() const { return deltaTime; }
	const float getTotalTimeSinceStart() const { return totalTimeSinceStart; }
	const float getTimeScale() const { return timeScale; }

    void updateDeltaTime();
	void reduceTimeScale() { timeScale *= 0.5f; }
	void increaseTimeScale() { timeScale *= 2.0f; }

    void updateObjectDescriptorSetsWithShadowMap();


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

    void createGlobeDescriptorSetLayout();
    void createGlobeDescriptorSets(const Engine::Texture& albedoTex);

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
    void cleanFences(std::vector<VkFence>& fences) const;
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

    VkDescriptorSetLayout globeDescriptorSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> globeDescriptorSets;
    Engine::Texture globeAlbedoTexture; // loaded during initialization (non-owning handles are stored in Texture)

    // --- Core Application Members ---
    Engine::Window window;
    Engine::InputHandler inputHandler;
    int numCacti = 1;

    std::vector<Engine::Object> objects;
	std::vector<Engine::ParticleSystem> particleSystems;
    Engine::GUI gui;
	int shadowObjectIndex = -1;

    // --- Lighting ---
	Engine::LightSource sunLight;
    Engine::LightSource moonLight;

    // --- Swapchain ---
    Engine::Swapchain swapChain;

    // --- Graphics Pipeline ---
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	Engine::SkyboxPipeline skyboxPipeline;
	VkDescriptorSetLayout skyboxDescriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> skyboxDescriptorSets;
	VkImageView skyboxCubemapView = VK_NULL_HANDLE;
	VkSampler skyboxCubemapSampler = VK_NULL_HANDLE;
    Engine::Mesh skyboxMesh;
    
    VkDescriptorSetLayout particleDescriptorSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> particleDescriptorSets;

	VkImage skyboxCubeImage = VK_NULL_HANDLE;
	VkDeviceMemory skyboxCubemapMemory = VK_NULL_HANDLE;

	Engine::Object* shadowCasterObject = nullptr;
    Engine::Object shadowObject;

    glm::vec3 globeCenter{ 0.0f, 0.0f, 0.0f };
    float globeRadius = 100.0f;

    Engine::Pipeline outsidePipeline;
	Engine::Pipeline insidePipeline;

    // Simulated solar parameters
    const float dayLengthSeconds = 60.0f;                // one simulated "day" = 60 seconds (adjust as needed)
    const float angularSpeed = glm::two_pi<float>() / dayLengthSeconds; // radians per second
    const float earthTilt = glm::radians(23.44f);        // Earth's axial tilt ~23.44 degrees
    const float orbitRadius = 200.0f;                    // distance from origin (tweak for visual scale)
    const float startAngle = glm::radians(-90.0f);       // starting angle (sunrise orientation)


    // --- shadow mapping resources ---
    VkImage shadowMapImage = VK_NULL_HANDLE;
    VkDeviceMemory shadowMapMemory = VK_NULL_HANDLE;
    VkImageView shadowMapView = VK_NULL_HANDLE;
    VkSampler shadowMapSampler = VK_NULL_HANDLE;

    const uint32_t SHADOW_MAP_SIZE = 4096; // Resolution of shadow map
	Engine::ShadowMapPipeline shadowMapPipeline;


    void createShadowMapResources();
    void cleanupShadowMapResources();

    // --- Camera ---
    Engine::Camera C1;
    Engine::Camera C2;
    Engine::Camera C3;
    Engine::Camera* camera = &C2;

    // --- Particle Management ---
    Engine::UniformBufferObject lastUBO;
    Engine::ParticleManager particleManager;

    // --- Buffers ---
    std::vector<Engine::Buffer> uniformBuffers;
    Engine::Buffer instanceBuffer;
    std::vector<Engine::InstanceData> instanceData;

    // --- Descriptors ---
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

	VkRenderPass imguiRenderPass = VK_NULL_HANDLE;

    // --- Synchronization ---
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

	float deltaTime = 0.0f;
	float correctedDeltaTime = 0.0f;
	float totalTimeSinceStart = 0.0f;
    float timeScale = 1.0f;
};