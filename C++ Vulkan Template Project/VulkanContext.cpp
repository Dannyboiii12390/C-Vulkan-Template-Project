#include "VulkanContext.h"
#include "Graphics/VulkanTypes.h"
#include "Graphics/Swapchain.h"
#include "Core/ModelLoader.h"
#include "Core/Flattener.h"
#include "Helpers.h"
#include <algorithm>
#include <array>
#include <set>
#include <chrono>
#include <unordered_map>
#include <fstream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

 // --- Main Application Flow ---
VulkanContext::VulkanContext() : window(1280, 720, "Vulkan 3D Application"), inputHandler(window)
{
    // Basic Vulkan setup
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();

    swapChain.create(*this);

    createDescriptorSetLayout();
    createCommandPool();


    // Uniform buffers (one per swapchain image)
    uniformBuffers.clear();
    uniformBuffers.reserve(swapChain.getImageCount());
    for (size_t i = 0; i < swapChain.getImageCount(); i++) {
        uniformBuffers.emplace_back(Engine::Buffer::createUniformBuffer(*this, sizeof(Engine::UniformBufferObject)));
    }

    // Skybox descriptor set layout must exist before creating skybox resources
    createSkyboxDescriptorSetLayout();

    // Descriptor pool used by all objects / skybox / particles
    createDescriptorPool();
    createShadowMapResources();

    // Load skybox cubemap (sampler + images) and create per-image descriptor sets for skybox
    auto [cubeSampler, cubeImages] = Engine::ModelLoader::LoadCubemapForSkybox(*this);
    skyboxCubemapView = cubeImages[0].imageView;
    skyboxCubemapSampler = cubeSampler;
    skyboxCubeImage = cubeImages[0].image;
    skyboxCubemapMemory = cubeImages[0].imageMemory;
    createSkyboxDescriptorSets(cubeImages[0].imageView, cubeSampler);

    skyboxMesh = Engine::ModelLoader::createCubeWithoutIndex(*this);

    createGlobeDescriptorSetLayout();
    globeAlbedoTexture = Engine::ModelLoader::createTextureImage(*this, "Objects/sky.jpg", true);//Engine::ModelLoader::createTextureImage(*this, "Objects/sand_alb.jpg", true);
	createGlobeDescriptorSets(globeAlbedoTexture);

    glm::vec3 sunDir;
    glm::vec3 moonDir;

    Engine::ModelLoader::loadFromConfig(*this, "config.txt", descriptorSetLayout, descriptorPool, uniformBuffers, objects, particleSystems, globeCenter, globeRadius, C1, C2, C3, particleManager, sunDir, moonDir);

    {
		// for testing: create a simple quad object
		Engine::flattenOBJTo2D("Objects/sandstone_rock.obj", "Objects/sandstone_rock_flattened.obj", Engine::FlattenMode::ProjectXZ, true);
		Engine::Mesh quadMesh = Engine::ModelLoader::loadOBJ(*this, "Objects/sandstone_rock_flattened.obj", 1.0f);
		Engine::Pipeline quadPipeline;
		quadPipeline.create(*this, Engine::ModelLoader::defaultVertexShaderPath, Engine::ModelLoader::defaultFragmentShaderPath, swapChain.getImageFormat(), swapChain.getDepthFormat(), descriptorSetLayout);
        uint8_t darkPixel[4] = { 30, 30, 30};
        uint8_t normalPixel[4] = { 128, 128, 255, 255 }; // normal (R=0.5, G=0.5, B=1.0), NOT sRGB
        Engine::Texture quadTexture = Engine::ModelLoader::createTextureImageFromMemory(*this, darkPixel, 1, 1, true);
        Engine::Texture normalTex = Engine::ModelLoader::createTextureImageFromMemory(*this, normalPixel, 1, 1, false);

        shadowObject.setPosition(glm::vec3(10.0f, 0, -5.0f));
        shadowObject.setScale(glm::vec3(0.01f, 0.01f, 0.01f));
		// Rotate to lie flat on XZ plane
        auto RadToDeg = [](float radians) { return radians * (180.0f / glm::pi<float>()); };
        shadowObject.setRotation(glm::vec3(-RadToDeg(glm::half_pi<float>()), RadToDeg(glm::half_pi<float>()), 0.0f));
		shadowObject.create(*this, std::move(quadMesh), std::move(quadPipeline), descriptorSetLayout, descriptorPool, uniformBuffers, quadTexture, normalTex);
        shadowObjectIndex = static_cast<int>(objects.size()) - 1;
		quadTexture.destroy(device);
		normalTex.destroy(device);
    }

    particleSystems.back().setActive(true);

    createDescriptorSets();

    updateObjectDescriptorSetsWithShadowMap();

    outsidePipeline.create(*this,
        "shaders/globe.vert.spv", "shaders/globe.frag.spv",
        getSwapchain().getImageFormat(), getSwapchain().getDepthFormat(),
        descriptorSetLayout,
        VK_CULL_MODE_BACK_BIT,
        true);
  
    insidePipeline.create(*this,
        "shaders/globe.vert.spv", "shaders/globe.frag.spv",
        getSwapchain().getImageFormat(), getSwapchain().getDepthFormat(),
        descriptorSetLayout,
        VK_CULL_MODE_NONE,
        true);

    // Skybox pipeline depends on skybox descriptor set layout (created earlier)
    skyboxPipeline.create(*this, "shaders/skybox.vert.spv", "shaders/skybox.frag.spv", swapChain.getImageFormat(), swapChain.getDepthFormat(), skyboxDescriptorSetLayout);
    ASSERT(skyboxPipeline.getPipeline() != VK_NULL_HANDLE);
	shadowMapPipeline.create(*this, "shaders/shadow_map.vert.spv", "shaders/shadow_map.frag.spv", VK_FORMAT_UNDEFINED, // No color attachment
        VK_FORMAT_D32_SFLOAT, descriptorSetLayout, VK_CULL_MODE_FRONT_BIT, true);
    // ADD THIS ASSERTION to verify pipeline was created:
    ASSERT(shadowMapPipeline.getPipeline() != VK_NULL_HANDLE, "Shadow pipeline creation failed!");

    // === Particle System Setup ===
    // Create particle descriptor set layout (same as skybox - just UBO)
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo particleLayoutInfo{};
    particleLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    particleLayoutInfo.bindingCount = 1;
    particleLayoutInfo.pBindings = &uboLayoutBinding;

    // add near the end of createDescriptorSetLayout(), right after vkCreateDescriptorSetLayout:
    VkResult res = vkCreateDescriptorSetLayout(device, &particleLayoutInfo, nullptr, &particleDescriptorSetLayout);
    ASSERT(res == VK_SUCCESS);
    

	ASSERT(particleDescriptorSetLayout != VK_NULL_HANDLE);
	
	particleManager.init(&particleSystems, &objects);

    // Create particle descriptor sets
    std::vector<VkDescriptorSetLayout> particleLayouts(swapChain.getImageCount(), particleDescriptorSetLayout);
    VkDescriptorSetAllocateInfo particleAllocInfo{};
    particleAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    particleAllocInfo.descriptorPool = descriptorPool;
    particleAllocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.getImageCount());
    particleAllocInfo.pSetLayouts = particleLayouts.data();

    particleDescriptorSets.resize(swapChain.getImageCount());
    res = vkAllocateDescriptorSets(device, &particleAllocInfo, particleDescriptorSets.data());
    ASSERT(res == VK_SUCCESS);

    // Update particle descriptor sets
    for (size_t i = 0; i < swapChain.getImageCount(); ++i)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Engine::UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = particleDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    // Command buffers & sync objects
    createCommandBuffers();
    createSyncObjects();

    camera = &C2;

    // Light Setup
    sunLight.create(*this, Engine::LightSource::Type::Directional, glm::vec3(1.0f, 0.3f, 0.1f), glm::vec3(0.0f), 1.0f, descriptorSetLayout, descriptorPool, uniformBuffers);

    // Enable orbit using computed, physically-inspired parameters
    sunLight.enableOrbit(sunDir, orbitRadius, angularSpeed, earthTilt, startAngle);

    moonLight.create(*this, Engine::LightSource::Type::Directional, glm::vec3(0.6f, 0.6f, 1.0f), glm::vec3(0.0f), 0.5f, descriptorSetLayout, descriptorPool, uniformBuffers, "Objects/moon_alb.jpg");
    moonLight.enableOrbit(moonDir, orbitRadius, angularSpeed, earthTilt, startAngle + glm::pi<float>());

    {
        // imgui setup
        auto qIndices = findQueueFamilies(physicalDevice);
        uint32_t graphicsQueueFamily = qIndices.graphicsFamily.value();

        // Create a simple render pass compatible with the swapchain (used only by ImGui)
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChain.getImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // keep scene already rendered; or CLEAR if you want ImGui to clear
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkResult r = vkCreateRenderPass(device, &renderPassInfo, nullptr, &imguiRenderPass);
        ASSERT(r == VK_SUCCESS, "Failed to create ImGui render pass");

        gui.create(window.getGLFWwindow(), instance, physicalDevice, device, graphicsQueueFamily, graphicsQueue, VK_NULL_HANDLE, imguiRenderPass, swapChain.getImageCount());

    }

 //   for (const auto& obj : objects) {
 //       std::cout << obj.getName() << ": " << obj.getMesh().getVertexCount() << " vertices" << "\n";
	//}
 //   std::cout << std::endl;
}
void VulkanContext::cleanup() {
    vkDeviceWaitIdle(device);

    // Shutdown GUI while device is still valid
    gui.shutdown();


    // --- Cleanup synchronization objects first ---
    cleanSemaphores(renderFinishedSemaphores);
    cleanSemaphores(imageAvailableSemaphores);
    cleanFences(inFlightFences);

    // Destroy or release swapchain resources
    swapChain.destroy(device);

    // --- Cleanup per-object resources while device is still valid ---
    // Objects own textures, samplers and pipelines; make sure they are fully cleaned
    // before destroying device or other global pipeline/sampler objects.
	for (auto& obj : objects) {
        obj.cleanup(*this);
    }
	objects.clear();
    for (auto& system : particleSystems)
    {
        system.cleanup(*this);
    }
    particleSystems.clear();

    // Cleanup meshes (these may reference device resources)
	shadowObject.cleanup(*this);
    skyboxMesh.cleanup(*this);
	shadowMapPipeline.destroy(device);

    // --- Destroy pipelines owned by the context (safe now that objects cleaned) ---
    skyboxPipeline.destroy(device);
    if (globeDescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, globeDescriptorSetLayout, nullptr);
        globeDescriptorSetLayout = VK_NULL_HANDLE;
    }

    // Destroy globe albedo texture resources created earlier
    if (globeAlbedoTexture.imageView != VK_NULL_HANDLE || globeAlbedoTexture.image != VK_NULL_HANDLE || globeAlbedoTexture.anisotropicSampler != VK_NULL_HANDLE) {
        globeAlbedoTexture.destroy(device);
    }

    sunLight.cleanup(*this);
    moonLight.cleanup(*this);
	cleanupShadowMapResources();

    // --- Descriptor pools and layouts ---
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }

    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        descriptorSetLayout = VK_NULL_HANDLE;
    }

    if (skyboxDescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, skyboxDescriptorSetLayout, nullptr);
        skyboxDescriptorSetLayout = VK_NULL_HANDLE;
    }

    if (particleDescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, particleDescriptorSetLayout, nullptr);
        particleDescriptorSetLayout = VK_NULL_HANDLE;
    }

    // --- Clean up uniform buffers ---
    for (auto& buf : uniformBuffers) buf.destroy(device);
    uniformBuffers.clear();

    // Clean up instance buffer if it exists
    if (instanceBuffer.getBuffer() != VK_NULL_HANDLE) {
        instanceBuffer.destroy(device);
    }

    // Command pool
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
    }

    // --- Skybox resources (sampler/view/image/memory) ---
    if (skyboxCubemapSampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, skyboxCubemapSampler, nullptr);
        skyboxCubemapSampler = VK_NULL_HANDLE;
    }
    if (skyboxCubemapView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, skyboxCubemapView, nullptr);
        skyboxCubemapView = VK_NULL_HANDLE;
    }
    if (skyboxCubeImage != VK_NULL_HANDLE) {
        vkDestroyImage(device, skyboxCubeImage, nullptr);
        skyboxCubeImage = VK_NULL_HANDLE;
    }
    if (skyboxCubemapMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, skyboxCubemapMemory, nullptr);
        skyboxCubemapMemory = VK_NULL_HANDLE;
    }

    // --- Final device destroy (all child objects must be gone by now) ---
    if (device != VK_NULL_HANDLE) {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }

    // Debug messenger (instance object)
    if (enableValidationLayers && debugMessenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        debugMessenger = VK_NULL_HANDLE;
    }

    // Surface
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }

    // Instance
    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}
void VulkanContext::updateObjectDescriptorSetsWithShadowMap()
{
    for (auto& obj : objects)
    {
        auto& objDescSets = obj.getDescriptorSets();

        for (size_t i = 0; i < objDescSets.size(); i++)
        {
            // Shadow map descriptor
            VkDescriptorImageInfo shadowMapInfo{};
            shadowMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            shadowMapInfo.imageView = shadowMapView;
            shadowMapInfo.sampler = shadowMapSampler;

            VkWriteDescriptorSet shadowMapWrite{};
            shadowMapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            shadowMapWrite.dstSet = objDescSets[i];
            shadowMapWrite.dstBinding = 4; // Shadow map binding
            shadowMapWrite.dstArrayElement = 0;
            shadowMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            shadowMapWrite.descriptorCount = 1;
            shadowMapWrite.pImageInfo = &shadowMapInfo;

            vkUpdateDescriptorSets(device, 1, &shadowMapWrite, 0, nullptr);
        }
    }
}
// --- Vulkan Initialization Methods ---
void VulkanContext::createInstance()
{
    ASSERT(!(enableValidationLayers && !checkValidationLayerSupport()), "Validation layers requested, but not available!");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan 3D Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        // Declare debugCreateInfo as close as possible to its use.
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);
    ASSERT(res == VK_SUCCESS);
}
void VulkanContext::setupDebugMessenger() {
    if (!enableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    VkResult res = CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger);
    ASSERT(res == VK_SUCCESS);
}
void VulkanContext::createSurface() {
    VkResult res = glfwCreateWindowSurface(instance, window.getGLFWwindow(), nullptr, &surface);
    ASSERT(res == VK_SUCCESS, "Failed to create window surface!");
}
void VulkanContext::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	ASSERT(deviceCount > 0);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& local_device : devices) {
        if (isDeviceSuitable(local_device)) {
            physicalDevice = local_device;
            break;
        }
    }
	ASSERT(physicalDevice != VK_NULL_HANDLE);
}
void VulkanContext::createLogicalDevice() {
    Engine::QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // --- Enable required features ---
    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.features.fillModeNonSolid = VK_TRUE; // For wireframe
    deviceFeatures2.features.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceSynchronization2Features sync2Features{};
    sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    sync2Features.synchronization2 = VK_TRUE;

    // Chain features
    deviceFeatures2.pNext = &dynamicRenderingFeatures;
    dynamicRenderingFeatures.pNext = &sync2Features;
    sync2Features.pNext = nullptr;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &deviceFeatures2;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = nullptr;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
    VkResult res = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    ASSERT(res == VK_SUCCESS);

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}
void VulkanContext::createDescriptorSetLayout() {
    // UBO binding (binding = 0)
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    // Albedo samplers binding (binding = 1)
    VkDescriptorSetLayoutBinding albedoBinding{};
    albedoBinding.binding = 1;
    albedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoBinding.descriptorCount = 1;
    albedoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    albedoBinding.pImmutableSamplers = nullptr;

    // Height/displacement map binding (binding = 2) - used in vertex shader for displacement
    VkDescriptorSetLayoutBinding heightBinding{};
    heightBinding.binding = 2;
    heightBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    heightBinding.descriptorCount = 1;
    heightBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // Allow both stages
    heightBinding.pImmutableSamplers = nullptr;

    // Normal samplers binding (binding = 3) - matches shader expectation
    VkDescriptorSetLayoutBinding normalBinding{};
    normalBinding.binding = 3;
    normalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalBinding.descriptorCount = 1;
    normalBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
    normalBinding.pImmutableSamplers = nullptr;

    // NEW: Shadow map binding (binding = 4)
    VkDescriptorSetLayoutBinding shadowMapBinding{};
    shadowMapBinding.binding = 4;
    shadowMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadowMapBinding.descriptorCount = 1;
    shadowMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    shadowMapBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 5> bindings = {
        uboLayoutBinding, albedoBinding, heightBinding, normalBinding, shadowMapBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    // add near the end of createDescriptorSetLayout(), right after vkCreateDescriptorSetLayout:
    VkResult res = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);
    ASSERT(res == VK_SUCCESS);
    
}
void VulkanContext::createSkyboxDescriptorSetLayout() {
    // Binding 0: UBO
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    // Binding 1: combined image sampler (cubemap)
    VkDescriptorSetLayoutBinding cubemapBinding{};
    cubemapBinding.binding = 1;
    cubemapBinding.descriptorCount = 1;
    cubemapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    cubemapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    cubemapBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, cubemapBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    // create skybox descriptor set layout into the correct member
    VkResult res = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &skyboxDescriptorSetLayout);
    ASSERT(res == VK_SUCCESS);
   
}
void VulkanContext::createSkyboxDescriptorSets(VkImageView cubemapView, VkSampler cubemapSampler) {
    // Allocate descriptor sets (one per swapchain image)
    std::vector<VkDescriptorSetLayout> layouts(swapChain.getImageCount(), skyboxDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.getImageCount());
    allocInfo.pSetLayouts = layouts.data();

    skyboxDescriptorSets.resize(swapChain.getImageCount());
    VkResult res = vkAllocateDescriptorSets(device, &allocInfo, skyboxDescriptorSets.data());
    ASSERT(res == VK_SUCCESS);

    // Update each descriptor set
    for (size_t i = 0; i < swapChain.getImageCount(); ++i)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Engine::UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = cubemapView;
        imageInfo.sampler = cubemapSampler;

        VkWriteDescriptorSet uboWrite{};
        uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrite.dstSet = skyboxDescriptorSets[i];
        uboWrite.dstBinding = 0;
        uboWrite.dstArrayElement = 0;
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &bufferInfo;

        VkWriteDescriptorSet samplerWrite{};
        samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        samplerWrite.dstSet = skyboxDescriptorSets[i];
        samplerWrite.dstBinding = 1;
        samplerWrite.dstArrayElement = 0;
        samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerWrite.descriptorCount = 1;
        samplerWrite.pImageInfo = &imageInfo;

        std::array<VkWriteDescriptorSet, 2> writes = { uboWrite, samplerWrite };
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}

void VulkanContext::createCommandPool() {
    Engine::QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	VkResult res = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
    ASSERT(res == VK_SUCCESS);
}
void VulkanContext::createDescriptorPool() {
    
    uint32_t n = static_cast<uint32_t>(swapChain.getImageCount());

    // Objects that use the main descriptor layout:
    //  - main mesh (1)
    //  - terrainObject (1)
    //  - sandstoneRockObject (1)
    //  - grottoObject (1)
    //  - cactus (1)
	//  - particle systems (1)
    uint32_t mainObjectCount = 100; // adjust if you add more
    const uint32_t mainSets = n * mainObjectCount;
    const uint32_t skyboxSets = n;
    const uint32_t particleSets = n;
    const uint32_t globeSets = n;
    const uint32_t totalSets = mainSets + skyboxSets + particleSets + globeSets;

    const uint32_t totalUBOs = totalSets; // 1 UBO per set
    const uint32_t mainSamplers = mainSets * 4;
    const uint32_t skyboxSamplers = skyboxSets * 1; // cubemap
    const uint32_t globeSamplers = globeSets * 2; // globe albedo + globe cubemap
    const uint32_t totalSamplers = mainSamplers + skyboxSamplers + globeSamplers;

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = totalUBOs;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = totalSamplers;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // Allow freeing individual descriptor sets (vkFreeDescriptorSets)
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = totalSets;

    VkResult res = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
    ASSERT(res == VK_SUCCESS);
}
void VulkanContext::createDescriptorSets() {

    descriptorSets.resize(swapChain.getImageCount());

    std::vector<VkDescriptorSetLayout> layouts(swapChain.getImageCount(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.getImageCount());
    allocInfo.pSetLayouts = layouts.data();

    ASSERT(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) == VK_SUCCESS);

    // Find a source of valid albedo/normal images for filling the "default" descriptorSets.
    // Prefer the first object that has non-null imageView/sampler; otherwise use skybox as fallback.
    VkDescriptorImageInfo defaultAlbedoInfo{};
    VkDescriptorImageInfo defaultNormalInfo{};
    bool foundValidObjectImages = false;

    for (const auto& o : objects)
    {
        auto [aInfo, nInfo] = o.getImages();
        if (aInfo.imageView != VK_NULL_HANDLE && aInfo.sampler != VK_NULL_HANDLE &&
            nInfo.imageView != VK_NULL_HANDLE && nInfo.sampler != VK_NULL_HANDLE)
        {
            defaultAlbedoInfo = aInfo;
            defaultNormalInfo = nInfo;
            foundValidObjectImages = true;
            break;
        }
    }

    // fallback: use skybox cubemap as placeholder (converted to a 2D-like descriptor) or a neutral texture
    if (!foundValidObjectImages)
    {
        // Use skybox as a safe placeholder for the height/normal binding if nothing else is available.
        defaultAlbedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        defaultAlbedoInfo.imageView = skyboxCubemapView;
        defaultAlbedoInfo.sampler = skyboxCubemapSampler;

        defaultNormalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        defaultNormalInfo.imageView = skyboxCubemapView;
        defaultNormalInfo.sampler = skyboxCubemapSampler;
    }

    for (size_t i = 0; i < swapChain.getImageCount(); i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Engine::UniformBufferObject);

        std::vector<VkDescriptorImageInfo> albedoInfos;
        std::vector<VkDescriptorImageInfo> normalInfos;

        albedoInfos.push_back(defaultAlbedoInfo);
        normalInfos.push_back(defaultNormalInfo);

        // Defensive validation: ensure imageView and sampler are valid before writing descriptors.
        if (albedoInfos[0].imageView == VK_NULL_HANDLE || albedoInfos[0].sampler == VK_NULL_HANDLE ||
            normalInfos[0].imageView == VK_NULL_HANDLE || normalInfos[0].sampler == VK_NULL_HANDLE) {
            throw std::runtime_error("createDescriptorSets(): texture imageView or sampler is VK_NULL_HANDLE — resources not ready before descriptor update");
        }

        // Height/displacement map (binding = 2) - using skybox for now as placeholder
        VkDescriptorImageInfo heightInfo{};
        heightInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        heightInfo.imageView = skyboxCubemapView;
        heightInfo.sampler = skyboxCubemapSampler;

        // NEW: Shadow map descriptor
        VkDescriptorImageInfo shadowMapInfo{};
        shadowMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadowMapInfo.imageView = shadowMapView;
        shadowMapInfo.sampler = shadowMapSampler;

        VkWriteDescriptorSet uboWrite{};
        uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrite.dstSet = descriptorSets[i];
        uboWrite.dstBinding = 0;
        uboWrite.dstArrayElement = 0;
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &bufferInfo;

        VkWriteDescriptorSet albedoWrite{};
        albedoWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        albedoWrite.dstSet = descriptorSets[i];
        albedoWrite.dstBinding = 1;
        albedoWrite.dstArrayElement = 0;
        albedoWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        albedoWrite.descriptorCount = static_cast<uint32_t>(albedoInfos.size());
        albedoWrite.pImageInfo = albedoInfos.data();

        VkWriteDescriptorSet heightWrite{};
        heightWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        heightWrite.dstSet = descriptorSets[i];
        heightWrite.dstBinding = 2;
        heightWrite.dstArrayElement = 0;
        heightWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        heightWrite.descriptorCount = 1;
        heightWrite.pImageInfo = &heightInfo;

        VkWriteDescriptorSet normalWrite{};
        normalWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        normalWrite.dstSet = descriptorSets[i];
        normalWrite.dstBinding = 3;
        normalWrite.dstArrayElement = 0;
        normalWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        normalWrite.descriptorCount = static_cast<uint32_t>(normalInfos.size());
        normalWrite.pImageInfo = normalInfos.data();

        // NEW: Shadow map write
        VkWriteDescriptorSet shadowMapWrite{};
        shadowMapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        shadowMapWrite.dstSet = descriptorSets[i];
        shadowMapWrite.dstBinding = 4; // Binding 4 for shadow map
        shadowMapWrite.dstArrayElement = 0;
        shadowMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        shadowMapWrite.descriptorCount = 1;
        shadowMapWrite.pImageInfo = &shadowMapInfo;

        std::array<VkWriteDescriptorSet, 5> descriptorWrites = {
            uboWrite, albedoWrite, heightWrite, normalWrite, shadowMapWrite };

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}
void VulkanContext::createCommandBuffers() {
    commandBuffers.resize(swapChain.getImageCount());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    VkResult res = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
    ASSERT(res == VK_SUCCESS);
}
void VulkanContext::createSyncObjects() {
    imageAvailableSemaphores.resize(swapChain.getImageCount());
    renderFinishedSemaphores.resize(swapChain.getImageCount());
    inFlightFences.resize(swapChain.getImageCount());

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < swapChain.getImageCount(); i++) {
        VkResult res = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
        ASSERT(res == VK_SUCCESS);
        res = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
        ASSERT(res == VK_SUCCESS);
		res = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]);
        ASSERT(res == VK_SUCCESS);
    }
}
void VulkanContext::drawFrame() {
	uint32_t currentFrame = window.getCurrentFrame();
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain.getSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
		swapChain.recreate(*this);
        return;
    }
    else 
    {
		ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
    }

    handleInput();
    updateUniformBuffer(imageIndex);
    particleManager.update(getDeltaTime() * getTimeScale(), lastUBO);

    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    VkCommandBufferSubmitInfo commandBufferInfo{};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferInfo.commandBuffer = commandBuffers[currentFrame];

    VkSemaphoreSubmitInfo waitSemaphoreInfo{};
    waitSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    waitSemaphoreInfo.semaphore = imageAvailableSemaphores[currentFrame];
    waitSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSemaphoreSubmitInfo signalSemaphoreInfo{};
    signalSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signalSemaphoreInfo.semaphore = renderFinishedSemaphores[currentFrame];
    signalSemaphoreInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreInfo;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferInfo;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalSemaphoreInfo;


	VkResult res = vkQueueSubmit2(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

    ASSERT(res == VK_SUCCESS);

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
    VkSwapchainKHR swapChains[] = { swapChain.getSwapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasFramebufferResized()) swapChain.recreate(*this);
    else ASSERT(result == VK_SUCCESS);

    window.resetCurrentFrame(swapChain.getImageCount());
}

void VulkanContext::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    auto currentFrame = window.getCurrentFrame();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    ASSERT(res == VK_SUCCESS);

    // ========== SHADOW PASS (NEW) ==========
    {
        // Transition shadow map to depth attachment layout
        VkImageMemoryBarrier2 shadowBarrierToDepth{};
        shadowBarrierToDepth.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        shadowBarrierToDepth.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        shadowBarrierToDepth.srcAccessMask = 0;
        shadowBarrierToDepth.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        shadowBarrierToDepth.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        shadowBarrierToDepth.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        shadowBarrierToDepth.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowBarrierToDepth.image = shadowMapImage;
        shadowBarrierToDepth.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        shadowBarrierToDepth.subresourceRange.levelCount = 1;
        shadowBarrierToDepth.subresourceRange.layerCount = 1;

        VkDependencyInfo shadowDepInfo{};
        shadowDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        shadowDepInfo.imageMemoryBarrierCount = 1;
        shadowDepInfo.pImageMemoryBarriers = &shadowBarrierToDepth;
        vkCmdPipelineBarrier2(commandBuffer, &shadowDepInfo);

        // Begin shadow rendering (depth only, no color attachment)
        VkRenderingAttachmentInfo shadowDepthAttachment{};
        shadowDepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        shadowDepthAttachment.imageView = shadowMapView;
        shadowDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowDepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        shadowDepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        shadowDepthAttachment.clearValue.depthStencil = { 1.0f, 0 };

        VkRenderingInfo shadowRenderingInfo{};
        shadowRenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        shadowRenderingInfo.renderArea = { {0, 0}, {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE} };
        shadowRenderingInfo.layerCount = 1;
        shadowRenderingInfo.colorAttachmentCount = 0; // No color attachments
        shadowRenderingInfo.pDepthAttachment = &shadowDepthAttachment;

        vkCmdBeginRendering(commandBuffer, &shadowRenderingInfo);

        VkViewport shadowViewport{};
        shadowViewport.width = static_cast<float>(SHADOW_MAP_SIZE);
        shadowViewport.height = static_cast<float>(SHADOW_MAP_SIZE);
        shadowViewport.minDepth = 0.0f;
        shadowViewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &shadowViewport);

        VkRect2D shadowScissor{ {0, 0}, {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE} };
        vkCmdSetScissor(commandBuffer, 0, 1, &shadowScissor);

        // bind the shadow pipeline once
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapPipeline.getPipeline());

        // Render shadow-casting objects (set small depth bias per-object to avoid over-biasing)
        for (auto& obj : objects) {
            if (!obj.isActive()) continue;

            const std::string_view name = obj.getName();
            if (name == "GlobeGlass") continue;

            // FIXED: Re-enable depth bias with appropriate values for your scene scale
            // These values prevent shadow acne while minimizing peter-panning
            vkCmdSetDepthBias(commandBuffer, 1.25f /* constantFactor */, 0.0f /* clamp */, 1.75f /* slopeFactor */);

            // Bind object's descriptor sets for UBO access (lightSpaceMatrix is in the UBO)
            const auto& descSets = obj.getDescriptorSets();
            if (!descSets.empty()) {
                vkCmdBindDescriptorSets(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    shadowMapPipeline.getLayout(), // Use shadow pipeline's layout
                    0, 1,
                    &descSets[currentFrame],
                    0, nullptr
                );
            }

            // Push model matrix using shadow pipeline's layout
            glm::mat4 modelMat = obj.getTransformMatrix();
            vkCmdPushConstants(
                commandBuffer,
                shadowMapPipeline.getLayout(), // Use shadow pipeline's layout
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(glm::mat4),
                &modelMat
            );

            // Bind and draw the mesh
            obj.getMesh().bind(commandBuffer);
            obj.getMesh().draw(commandBuffer);
        }


        vkCmdEndRendering(commandBuffer);

        // Transition shadow map to shader read layout
        VkImageMemoryBarrier2 shadowBarrierToShader{};
        shadowBarrierToShader.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        shadowBarrierToShader.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        shadowBarrierToShader.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        shadowBarrierToShader.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        shadowBarrierToShader.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        shadowBarrierToShader.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        shadowBarrierToShader.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shadowBarrierToShader.image = shadowMapImage;
        shadowBarrierToShader.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        shadowBarrierToShader.subresourceRange.levelCount = 1;
        shadowBarrierToShader.subresourceRange.layerCount = 1;

        VkDependencyInfo shadowShaderDep{};
        shadowShaderDep.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        shadowShaderDep.imageMemoryBarrierCount = 1;
        shadowShaderDep.pImageMemoryBarriers = &shadowBarrierToShader;
        vkCmdPipelineBarrier2(commandBuffer, &shadowShaderDep);
    }

    VkImageMemoryBarrier2 imageBarrierToAttachment{};
    imageBarrierToAttachment.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrierToAttachment.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    imageBarrierToAttachment.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageBarrierToAttachment.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    imageBarrierToAttachment.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrierToAttachment.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageBarrierToAttachment.image = swapChain.getImage(imageIndex);
    imageBarrierToAttachment.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo dependencyInfoToAttachment{};
    dependencyInfoToAttachment.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfoToAttachment.imageMemoryBarrierCount = 1;
    dependencyInfoToAttachment.pImageMemoryBarriers = &imageBarrierToAttachment;
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfoToAttachment);

    if (swapChain.getDepthImage() != VK_NULL_HANDLE)
    {
        VkImageMemoryBarrier2 depthBarrier{};
        depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        depthBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        depthBarrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        depthBarrier.srcAccessMask = 0;
        depthBarrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthBarrier.image = swapChain.getDepthImage();
        depthBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
        if (swapChain.getDepthFormat() == VK_FORMAT_D32_SFLOAT_S8_UINT || swapChain.getDepthFormat() == VK_FORMAT_D24_UNORM_S8_UINT)
            depthBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

        VkDependencyInfo depthDep{};
        depthDep.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depthDep.imageMemoryBarrierCount = 1;
        depthDep.pImageMemoryBarriers = &depthBarrier;
        vkCmdPipelineBarrier2(commandBuffer, &depthDep);
    }

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = swapChain.getImageView(imageIndex);
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = swapChain.getDepthImageView();
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = { {0, 0}, swapChain.getExtent()};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;

    VkViewport viewport{};
    viewport.width = static_cast<float>(swapChain.getExtent().width);
    viewport.height = static_cast<float>(swapChain.getExtent().height);
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent = swapChain.getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    // Skybox rendering
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.getPipeline());
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.getLayout(), 0, 1, &skyboxDescriptorSets[currentFrame], 0, nullptr);
    
    Engine::PushConstantModel skyPC{};
    skyPC.model = glm::mat4(1.0f);
    vkCmdPushConstants(commandBuffer, skyboxPipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Engine::PushConstantModel), &skyPC);
    skyboxMesh.bind(commandBuffer);
    skyboxMesh.draw(commandBuffer);

    {
        glm::vec3 sunDir = sunLight.getDirection(); // normalized ray expected (rays direction)
        glm::vec3 originalPos = glm::vec3(10.0f, -0.49f, -5.0f); // object's base position
        float meshHeight = 1.0f;
        float meshRadius = 0.5f;

        glm::vec2 shadowScaleXZ, centerOffsetXZ;
        Helpers::computeShadowProjection(sunDir, originalPos, meshHeight, meshRadius, shadowScaleXZ, centerOffsetXZ, 1.0f);

        // Compute yaw so quad's long axis aligns with shadow direction
        float L = glm::length(centerOffsetXZ) * 2.0f; // approximate projected length
        glm::vec2 dirForYaw = (L > 1e-5f) ? glm::normalize(centerOffsetXZ * 2.0f) : glm::vec2(0.0f, -1.0f);
        float yawDeg = glm::degrees(atan2(dirForYaw.x, dirForYaw.y));

        const glm::vec3 baseScale(0.01f, 0.01f, 0.01f);
        const glm::vec3 finalScale = glm::vec3(baseScale.x * shadowScaleXZ.x, baseScale.y, baseScale.z * shadowScaleXZ.y);
        const glm::vec3 finalPos = originalPos + glm::vec3(centerOffsetXZ.x, 0.0f, centerOffsetXZ.y);
        const glm::vec3 finalRot = glm::vec3(-90.0f, yawDeg + 90.0f, 0.0f); // adjust as needed for mesh axis

        shadowObject.setScale(finalScale);
        shadowObject.setRotation(finalRot);
        shadowObject.setPosition(finalPos);
        
    }
    sunLight.draw(commandBuffer, currentFrame);
    moonLight.draw(commandBuffer, currentFrame);
    
    // drawing objects
    // todo to change
    // drawing objects
    for (auto& obj : objects) {
        if (!obj.isActive()) continue;

        const std::string_view name = obj.getName();

        if (name == Engine::ModelLoader::globe)
        {
            // choose pipeline based on whether camera is inside the globe (updated in updateUniformBuffer)
            bool inside = (lastUBO.inside_globe != 0);

            if (inside) {
                // bind inside pipeline and globe descriptor set(s) manually
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, insidePipeline.getPipeline());

                const auto& descSets = obj.getDescriptorSets();
                ASSERT(!descSets.empty());
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, insidePipeline.getLayout(), 0, 1, &descSets[currentFrame], 0, nullptr);

                // push model matrix (same as Object::draw does)
                glm::mat4 modelMat = obj.getTransformMatrix();
                vkCmdPushConstants(commandBuffer, insidePipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(modelMat), &modelMat);

                // bind mesh and draw
                obj.getMesh().bind(commandBuffer);
                obj.getMesh().draw(commandBuffer);
            }

            else
            {
                // bind inside pipeline and globe descriptor set(s) manually
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, outsidePipeline.getPipeline());

                const auto& descSets = obj.getDescriptorSets();
                ASSERT(!descSets.empty());
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, outsidePipeline.getLayout(), 0, 1, &descSets[currentFrame], 0, nullptr);

                // push model matrix (same as Object::draw does)
                glm::mat4 modelMat = obj.getTransformMatrix();
                vkCmdPushConstants(commandBuffer, outsidePipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(modelMat), &modelMat);

                // bind mesh and draw
                obj.getMesh().bind(commandBuffer);
                obj.getMesh().draw(commandBuffer);
            }
        }
        else
        {
            // Default: draw using each object's own pipeline/descriptor sets
            obj.draw(commandBuffer, currentFrame);
        }
    };

	// Particle system rendering
    for(auto& particleSystem : particleSystems) {
        if (particleSystem.isActive())
        {
            particleSystem.update(getDeltaTime()* getTimeScale());
            particleSystem.draw(commandBuffer, currentFrame);
        }
	}
	shadowObject.draw(commandBuffer, currentFrame);

    // Start new ImGui frame
    gui.newFrame();

    // Build your UI
    ImGui::Begin("Statistics & Controls");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("snow_probability: %.3f", particleManager.getSnowProbability());
	ImGui::Text("sun_pos : (% .2f, % .2f, % .2f)", sunLight.getPosition().x, sunLight.getPosition().y, sunLight.getPosition().z);
	ImGui::Text("moon_pos : (% .2f, % .2f, % .2f)", moonLight.getPosition().x, moonLight.getPosition().y, moonLight.getPosition().z);

    // Add controls here
    if (ImGui::Button("Select C1")) {
        camera = &C1;
    }
    if (ImGui::Button("Select C2")) {
        camera = &C2;
    }
    if (ImGui::Button("Select C3")) {
        camera = &C3;
    }
    if (ImGui::Button("Toggle Rain")) {
		particleSystems[1].setActive(!particleSystems[1].isActive()); // Toggle rain particle system
    }
    if (ImGui::Button("Toggle Snow")) {
        particleSystems[2].setActive(!particleSystems[2].isActive()); // Toggle rain particle system
    }
    if (ImGui::Button("Toggle Fire")) {
        particleSystems[3].setActive(!particleSystems[3].isActive()); // Toggle rain particle system
    }
    if (ImGui::Button("Toggle Dust")) {
        particleSystems[4].setActive(!particleSystems[4].isActive()); // Toggle rain particle system
    }

    ImGui::End();

    // Render ImGui (inside your command buffer recording)
    gui.render(commandBuffer);

    vkCmdEndRendering(commandBuffer);

    VkImageMemoryBarrier2 imageBarrierToPresent{};
    imageBarrierToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrierToPresent.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageBarrierToPresent.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    imageBarrierToPresent.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    imageBarrierToPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrierToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imageBarrierToPresent.image = swapChain.getImage(imageIndex);
    imageBarrierToPresent.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo dependencyInfoToPresent{};
    dependencyInfoToPresent.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfoToPresent.imageMemoryBarrierCount = 1;
    dependencyInfoToPresent.pImageMemoryBarriers = &imageBarrierToPresent;
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfoToPresent);

    res = vkEndCommandBuffer(commandBuffer);
    ASSERT(res == VK_SUCCESS);
}
void VulkanContext::createShadowMapResources() {
    // Create shadow map image (depth only)
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = SHADOW_MAP_SIZE;
    imageInfo.extent.height = SHADOW_MAP_SIZE;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_D32_SFLOAT; // Depth format
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    ASSERT(vkCreateImage(device, &imageInfo, nullptr, &shadowMapImage) == VK_SUCCESS);

    // Allocate memory using existing Buffer method pattern
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, shadowMapImage, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    uint32_t memoryTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            memoryTypeIndex = i;
            break;
        }
    }
    ASSERT(memoryTypeIndex != UINT32_MAX);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &shadowMapMemory) == VK_SUCCESS);
    vkBindImageMemory(device, shadowMapImage, shadowMapMemory, 0);

    // Create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = shadowMapImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_D32_SFLOAT;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    ASSERT(vkCreateImageView(device, &viewInfo, nullptr, &shadowMapView) == VK_SUCCESS);

    // Create sampler for shadow map (with comparison for PCF)
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    // Use CLAMP_TO_EDGE so PCF samples near the border don't read the "border color"
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

    // borderColor is ignored for CLAMP_TO_EDGE, but left configured defensively
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    // Enable depth comparison for sampler2DShadow usage
    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &shadowMapSampler) == VK_SUCCESS);
}

void VulkanContext::cleanupShadowMapResources() {
    if (shadowMapSampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, shadowMapSampler, nullptr);
        shadowMapSampler = VK_NULL_HANDLE;
    }
    if (shadowMapView != VK_NULL_HANDLE) {
        vkDestroyImageView(device, shadowMapView, nullptr);
        shadowMapView = VK_NULL_HANDLE;
    }
    if (shadowMapImage != VK_NULL_HANDLE) {
        vkDestroyImage(device, shadowMapImage, nullptr);
        shadowMapImage = VK_NULL_HANDLE;
    }
    if (shadowMapMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, shadowMapMemory, nullptr);
        shadowMapMemory = VK_NULL_HANDLE;
    }
}
void VulkanContext::updateUniformBuffer(uint32_t currentImage)
{
    Engine::UniformBufferObject ubo = camera->getCameraUBO();

    const float time = getTotalTimeSinceStart();

    sunLight.update(time);
    sunLight.applyToUBO(ubo, 0);
    moonLight.update(time);
    moonLight.applyToUBO(ubo, 1);

    ubo.time = time;

    float distToCenter = glm::length(ubo.eyePos - globeCenter);
    bool inside = distToCenter < (globeRadius * 0.99f);
    ubo.inside_globe = inside ? 1 : 0;

    // IMPROVED: Calculate light space matrix for shadow mapping
    auto getLightSpaceMatrix = [](auto lightDir) {
        // CRITICAL FIX: Focus shadow frustum on the actual objects in your scene
        // Instead of centering on globeCenter (which might be far away),
        // center on where your visible objects are
        glm::vec3 sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f); // Adjust to match your scene's center

        // If you have a specific area of interest (like around the camera), use that instead:
        // glm::vec3 sceneCenter = ubo.eyePos; // Follow the camera

        // Position the "light camera" far back along the light direction
        float lightDistance = 150.0f; // Increased distance
        glm::vec3 lightPos = sceneCenter - lightDir * lightDistance;

        // Create the light view matrix looking toward the scene center
        glm::mat4 lightView = glm::lookAt(
            lightPos,
            sceneCenter,
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // CRITICAL: Adjust these values to match your actual scene bounds
        // Start with smaller values and increase if shadows are cut off
        float shadowExtent = 100.0f;  // Try 50, 75, 100, 150 based on your scene
        float nearPlane = 1.0f;        // Increased from 0.1f to reduce precision issues
        float farPlane = 350.0f;       // Should be > lightDistance + sceneRadius

        glm::mat4 lightProjection = glm::ortho(
            -shadowExtent, shadowExtent,
            -shadowExtent, shadowExtent,
            nearPlane, farPlane
        );
		return std::make_tuple(lightProjection, lightView);
	};
    
	auto [lightProjection, lightView] = getLightSpaceMatrix(sunLight.getDirection());
	ubo.lightSpaceMatrix = lightProjection * lightView;

    lastUBO = ubo;

    uniformBuffers[currentImage].write(device, &ubo, sizeof(ubo));
}
void VulkanContext::createGlobeDescriptorSetLayout()
{
    // Binding 0: UBO
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    // Binding 1: globe albedo combined image sampler
    VkDescriptorSetLayoutBinding albedoBinding{};
    albedoBinding.binding = 1;
    albedoBinding.descriptorCount = 1;
    albedoBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    albedoBinding.pImmutableSamplers = nullptr;

    // Binding 2: cubemap sampler (environment)
    VkDescriptorSetLayoutBinding cubemapBinding{};
    cubemapBinding.binding = 2;
    cubemapBinding.descriptorCount = 1;
    cubemapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    cubemapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    cubemapBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, albedoBinding, cubemapBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VkResult res = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &globeDescriptorSetLayout);
    ASSERT(res == VK_SUCCESS);
}
void VulkanContext::createGlobeDescriptorSets(const Engine::Texture& albedoTex)
{
    // Allocate descriptor sets (one per swapchain image)
    globeDescriptorSets.resize(swapChain.getImageCount());
    std::vector<VkDescriptorSetLayout> layouts(swapChain.getImageCount(), globeDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.getImageCount());
    allocInfo.pSetLayouts = layouts.data();

    VkResult res = vkAllocateDescriptorSets(device, &allocInfo, globeDescriptorSets.data());
    ASSERT(res == VK_SUCCESS);

    // Prepare image infos for albedo and cubemap (use provided albedoTex and already-loaded skybox)
    for (size_t i = 0; i < swapChain.getImageCount(); ++i)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Engine::UniformBufferObject);

        // Albedo (binding = 1)
        VkDescriptorImageInfo albedoImageInfo{};
        albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoImageInfo.imageView = albedoTex.imageView;
        albedoImageInfo.sampler = getCurrentSampler(albedoTex);

        // Cubemap (binding = 2) - use skybox resources loaded earlier
        VkDescriptorImageInfo cubemapImageInfo{};
        cubemapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cubemapImageInfo.imageView = skyboxCubemapView;
        cubemapImageInfo.sampler = skyboxCubemapSampler;

        // Defensive checks
        ASSERT(bufferInfo.buffer != VK_NULL_HANDLE);
        ASSERT(albedoImageInfo.imageView != VK_NULL_HANDLE && albedoImageInfo.sampler != VK_NULL_HANDLE);
        ASSERT(cubemapImageInfo.imageView != VK_NULL_HANDLE && cubemapImageInfo.sampler != VK_NULL_HANDLE);

        VkWriteDescriptorSet uboWrite{};
        uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrite.dstSet = globeDescriptorSets[i];
        uboWrite.dstBinding = 0;
        uboWrite.dstArrayElement = 0;
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &bufferInfo;

        VkWriteDescriptorSet albedoWrite{};
        albedoWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        albedoWrite.dstSet = globeDescriptorSets[i];
        albedoWrite.dstBinding = 1;
        albedoWrite.dstArrayElement = 0;
        albedoWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        albedoWrite.descriptorCount = 1;
        albedoWrite.pImageInfo = &albedoImageInfo;

        VkWriteDescriptorSet cubemapWrite{};
        cubemapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        cubemapWrite.dstSet = globeDescriptorSets[i];
        cubemapWrite.dstBinding = 2;
        cubemapWrite.dstArrayElement = 0;
        cubemapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cubemapWrite.descriptorCount = 1;
        cubemapWrite.pImageInfo = &cubemapImageInfo;

        std::array<VkWriteDescriptorSet, 3> writes = { uboWrite, albedoWrite, cubemapWrite };
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}

// --- Helper Methods ---
bool VulkanContext::isDeviceSuitable(VkPhysicalDevice pDevice) const {
    Engine::QueueFamilyIndices indices = findQueueFamilies(pDevice);
    bool extensionsSupported = checkDeviceExtensionSupport(pDevice);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        Engine::SwapChainSupportDetails swapChainSupport = querySwapChainSupport(pDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &dynamicRenderingFeatures;
    vkGetPhysicalDeviceFeatures2(pDevice, &features2);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && dynamicRenderingFeatures.dynamicRendering;
}
bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice pDevice) const {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}
Engine::QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice pDevice) const {
    Engine::QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, queueFamilies.data());
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}
Engine::SwapChainSupportDetails VulkanContext::querySwapChainSupport(VkPhysicalDevice pDevice) const {
    Engine::SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, surface, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, surface, &formatCount, details.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}
VkSurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}
VkPresentModeKHR VulkanContext::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D VulkanContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window.getGLFWwindow(), &width, &height);
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}
std::vector<const char*> VulkanContext::getRequiredExtensions() const {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}
bool VulkanContext::checkValidationLayerSupport() const {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}
glm::mat4 VulkanContext::rotateAboutPoint(const glm::vec3& pivot, const float angle, const glm::vec3& axis)
{
    // Translate to origin
    glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -pivot);

    // Rotate around the provided axis
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);

    // Translate back to original position
    glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), pivot);

    // Combine the transformations
    glm::mat4 modelMatrix = translateBack * rotation * translateToOrigin;

    return modelMatrix;
}

void VulkanContext::handleInput()
{
    float sprintSpeed = 20.0f;

    // Map function keys to camera instances
    std::unordered_map<int, decltype(camera)> cameraMap = {
        { GLFW_KEY_F1, &C1 },
        { GLFW_KEY_F2, &C2 },
        { GLFW_KEY_F3, &C3 }
    };

    // Switch active camera if corresponding key is pressed
    for (const auto& [key, camPtr] : cameraMap)
    {
        if (inputHandler.isKeyPressed(key) && camPtr != nullptr)
        {
            camera = camPtr;
            break;
        }
    }
    if (inputHandler.isKeyPressed(GLFW_KEY_R)) 
    {
        glm::vec3 sunDir;
		glm::vec3 moonDir;
        Engine::ModelLoader::reloadFromConfig(*this, "config.txt", descriptorSetLayout, descriptorPool, uniformBuffers, objects, particleSystems, 
            globeCenter, globeRadius, C1, C2, C3, particleManager, sunDir, moonDir);
        sunLight.enableOrbit(sunDir, orbitRadius, angularSpeed, earthTilt, startAngle);
        moonLight.enableOrbit(moonDir, orbitRadius, angularSpeed, earthTilt, startAngle + glm::pi<float>());

        shadowObjectIndex = -1;
    }
        
    if (inputHandler.isKeyPressed(GLFW_KEY_N))
    {
        particleSystems[1].setActive(!particleSystems[1].isActive()); //toggle rain
		particleSystems[2].setActive(false); //disable snow
    }
	if (inputHandler.isKeyPressed(GLFW_KEY_F4)) particleSystems[3].setActive(!particleSystems[3].isActive());

    static bool prevTKey = false;
    bool tDown = inputHandler.isKeyPressed(GLFW_KEY_T);

    if (tDown && !prevTKey)
    {
        bool shiftDown = inputHandler.isKeyPressed(GLFW_KEY_LEFT_SHIFT);
        if (shiftDown)
        {
            reduceTimeScale();
        }
        else
        {
            increaseTimeScale();
        }
    }
    prevTKey = tDown;
    window.setShouldClose(inputHandler.isKeyPressed(GLFW_KEY_ESCAPE));

    // Compute movement speed; when Left Shift is pressed apply sprint multiplier.
    // Use static_cast<float> to convert the boolean/int returned by isKeyPressed into a float (0.0 or 1.0).
    float movespeed = 3.0f * getDeltaTime() * (1.0f + sprintSpeed * static_cast<float>(inputHandler.isKeyPressed(GLFW_KEY_LEFT_SHIFT)));

    camera->moveForward(movespeed * inputHandler.isKeyPressed(GLFW_KEY_W));
    camera->moveForward(-movespeed * inputHandler.isKeyPressed(GLFW_KEY_S));
    camera->moveRight(movespeed * inputHandler.isKeyPressed(GLFW_KEY_D));
    camera->moveRight(-movespeed * inputHandler.isKeyPressed(GLFW_KEY_A));
    camera->moveUp(movespeed * inputHandler.isKeyPressed(GLFW_KEY_SPACE));
    camera->moveUp(-movespeed * inputHandler.isKeyPressed(GLFW_KEY_LEFT_ALT));

    const float keyRotSpeed = 90.0f; // degrees (or units expected by camera->rotate) per second
    float keyYawDelta = keyRotSpeed * getDeltaTime ()* (static_cast<float>(inputHandler.isKeyPressed(GLFW_KEY_L)) - static_cast<float>(inputHandler.isKeyPressed(GLFW_KEY_J)));
    float keyPitchDelta = keyRotSpeed * getDeltaTime() * (static_cast<float>(inputHandler.isKeyPressed(GLFW_KEY_I)) - static_cast<float>(inputHandler.isKeyPressed(GLFW_KEY_K)));

    camera->rotate(keyYawDelta, keyPitchDelta);


    // Camera rotation with mouse
    double mouseX, mouseY;
    inputHandler.getMouseDelta(mouseX, mouseY);
    float sensitivity = 0.1f;
    camera->rotate(static_cast<float>(mouseX * sensitivity), static_cast<float>(-mouseY * sensitivity));

}
void VulkanContext::cleanFences(std::vector<VkFence>& fences) const
{
    for (VkFence& fence : fences) {
        if (fence != VK_NULL_HANDLE) {
            vkDestroyFence(device, fence, nullptr);
            fence = VK_NULL_HANDLE;
        }
    }
    fences.clear();
}
void VulkanContext::updateDeltaTime()
{
    using clock = std::chrono::high_resolution_clock;
    static auto last = clock::now();
    auto current = clock::now();

    std::chrono::duration<float> elapsed = current - last;
    last = current;

    // Unscaled frame time in seconds
    float unscaledDelta = elapsed.count();

    // Prevent negative/zero issues
    if (unscaledDelta < 0.0f) unscaledDelta = 0.0f;

    deltaTime = unscaledDelta;

    // Apply time scale to get corrected delta time
    correctedDeltaTime = deltaTime * timeScale;

    // Accumulate total time using the corrected (scaled) delta
    totalTimeSinceStart += correctedDeltaTime;
}

VkSampler VulkanContext::getCurrentSampler(const Engine::Texture& texture) const
{
    return texture.anisotropicSampler;
}
void VulkanContext::updateDescriptorSetsForTexture()
{
    // Wait for device to be idle before updating descriptors
    vkDeviceWaitIdle(device);

	const Engine::Texture& currentAlbedo = objects[0].getAlbedoTexture();
    const Engine::Texture& currentNormal = objects[0].getNormalTexture();

    VkSampler activeAlbedoSampler = getCurrentSampler(currentAlbedo);
    VkSampler activeNormalSampler = getCurrentSampler(objects[0].getNormalTexture());

    for (size_t i = 0; i < swapChain.getImageCount(); i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Engine::UniformBufferObject);

        VkDescriptorImageInfo albedoInfo{};
        albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoInfo.imageView = currentAlbedo.imageView;
        albedoInfo.sampler = activeAlbedoSampler;

        VkDescriptorImageInfo normalInfo{};
        normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalInfo.imageView = currentNormal.imageView;
        normalInfo.sampler = activeNormalSampler;

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &albedoInfo;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &normalInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanContext::createDescriptorSetsForTexture(const Engine::Texture& tex, std::vector<VkDescriptorSet>& descSets)
{
    std::vector<VkDescriptorSetLayout> layouts(swapChain.getImageCount(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.getImageCount());
    allocInfo.pSetLayouts = layouts.data();

    descSets.resize(swapChain.getImageCount());
    VkResult res = vkAllocateDescriptorSets(device, &allocInfo, descSets.data());
    ASSERT(res == VK_SUCCESS);

    // Update descriptor sets with texture (albedo only) - keep for compatibility
    for (size_t i = 0; i < swapChain.getImageCount(); i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Engine::UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = tex.imageView;
        imageInfo.sampler = getCurrentSampler(tex);

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
    }
}

