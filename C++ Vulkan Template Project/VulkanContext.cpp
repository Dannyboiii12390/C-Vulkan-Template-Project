#include "VulkanContext.h"
#include "Graphics/VulkanTypes.h"
#include "Graphics/Swapchain.h"

#include "Core/ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

constexpr int NUM_INSTANCES = 1;

void Object::draw(Engine::Pipeline& pipeline, VkCommandBuffer commandBuffer, float time, int positive)
{
    //positive = positive % 1;
	// Drawing logic for the object
        // Orbit parameters
    float orbitSpeedDegPerSec = 45.0f;   // degrees per second
    float orbitRadius = 3.0f;            // distance from terrain center
    glm::vec3 orbitAxis = glm::vec3(0.0f, 1.0f, 0.0f);

    Engine::PushConstantModel pushConstant{};
    mesh.bind(commandBuffer);
    //glm::mat4 meshScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 2.5f)); // adjust mesh scale as desired

    if (orbitingAround)
    {
        glm::mat4 offset = glm::translate(glm::mat4(1.0f), glm::vec3(orbitRadius, 0.0f, 0.0f));
        glm::mat4 orbitRotation = VulkanContext::rotateAboutPoint(orbitingAround->position, time * glm::radians(orbitSpeedDegPerSec)*positive, orbitAxis);

        Engine::PushConstantModel pushConstant{};
        mesh.bind(commandBuffer);
        pushConstant.model = orbitRotation * offset;
        vkCmdPushConstants(commandBuffer, pipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Engine::PushConstantModel), &pushConstant);
        glm::vec4 worldPos = pushConstant.model * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        position = glm::vec3(worldPos);
    }
    else
    {
        glm::mat4 orbitRotation = VulkanContext::rotateAboutPoint(position, time * glm::radians(orbitSpeedDegPerSec), orbitAxis);
        pushConstant.model = orbitRotation;
        vkCmdPushConstants(commandBuffer, pipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Engine::PushConstantModel), &pushConstant);
        mesh.draw(commandBuffer);
    }

    mesh.draw(commandBuffer);

}

// --- Main Application Flow ---
VulkanContext::VulkanContext() : window(1280, 720, "Vulkan 3D Application"), inputHandler(window)
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();

    swapChain.create(*this);

    createDescriptorSetLayout();

    // --- create graphics pipeline ---
    pipeline.create(*this, "shaders/shader.vert.spv", "shaders/shader.frag.spv", swapChain.imageFormat, descriptorSetLayout);

    createCommandPool();

	//terrain = Engine::ModelLoader::createCylinder(*this, 0.5f, 1.0f, 36);
    //terrain = Engine::ModelLoader::createSphere(*this, 1.0f, 36, 18);
    mesh = Engine::ModelLoader::createCube(*this);
    //mesh = Engine::ModelLoader::createCylinder(*this, 0.5f, 1.0f, 36);
    //mesh = Engine::ModelLoader::createGrid(*this, 20, 20);
    //mesh = Engine::ModelLoader::createTerrain(*this, 50, 50, 1.0f);
    //mesh = Engine::ModelLoader::loadObj(*this, "Objects/drone.obj");
	//mesh = Engine::ModelLoader::createSphere(*this, 1.0f, 36, 18);

    // --- create uniform buffers ---
    uniformBuffers.clear();
    uniformBuffers.reserve(swapChain.imageCount);

    for (size_t i = 0; i < swapChain.imageCount; i++) {
        uniformBuffers.emplace_back(Engine::Buffer::createUniformBuffer(*this, sizeof(Engine::UniformBufferObject)));
    }

    //should be in shader class
    createDescriptorPool();
    createDescriptorSets();

    createCommandBuffers();
    createSyncObjects();

    // --- Initialize camera ---
    camera.create(45.0f, static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight()), 0.1f, 100.0f);
    camera.setPosition(glm::vec3(0.0f, 5.0f, 10.0f));
    camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
}
void VulkanContext::cleanup() {

    ASSERT(device != VK_NULL_HANDLE); 
	vkDeviceWaitIdle(device);

	cleanSemaphores(renderFinishedSemaphores);
	cleanSemaphores(imageAvailableSemaphores);
	cleanFences(inFlightFences);

    swapChain.destroy(device);

    pipeline.destroy(device);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    mesh.cleanup(*this);
	//terrain.cleanup(*this);
	//obj1.mesh.cleanup(*this);
	//obj2.mesh.cleanup(*this);
	//obj3.mesh.cleanup(*this);

    for (auto& buf : uniformBuffers) buf.destroy(device);
    uniformBuffers.clear();
    instanceBuffer.destroy(device);

	// Descriptor pool
	ASSERT(descriptorPool != VK_NULL_HANDLE);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    descriptorPool = VK_NULL_HANDLE;

    // Command pool
	ASSERT(commandPool != VK_NULL_HANDLE);
	vkDestroyCommandPool(device, commandPool, nullptr);
	commandPool = VK_NULL_HANDLE;

    // Device
	ASSERT(device != VK_NULL_HANDLE);
	vkDestroyDevice(device, nullptr);
	device = VK_NULL_HANDLE;
    
    // Debug messenger (instance object)
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        debugMessenger = VK_NULL_HANDLE;
    }

    // Surface
	ASSERT(instance != VK_NULL_HANDLE);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    surface = VK_NULL_HANDLE;

	// Instance
	ASSERT(instance != VK_NULL_HANDLE);
    vkDestroyInstance(instance, nullptr);
	instance = VK_NULL_HANDLE;
}

// --- Vulkan Initialization Methods ---
void VulkanContext::createInstance() {
    ASSERT_MSG(!(enableValidationLayers && !checkValidationLayerSupport()), "Validation layers requested, but not available!");

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

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    ASSERT(vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS);
}
void VulkanContext::setupDebugMessenger() {
    if (!enableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    ASSERT(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) == VK_SUCCESS);
}
void VulkanContext::createSurface() {
    ASSERT_MSG((glfwCreateWindowSurface(instance, window.getGLFWwindow(), nullptr, &surface) == VK_SUCCESS), "Failed to create window surface!");
}
void VulkanContext::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	ASSERT(deviceCount > 0);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
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

    ASSERT(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) == VK_SUCCESS);

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}
void VulkanContext::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    ASSERT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) == VK_SUCCESS);
}
void VulkanContext::createCommandPool() {
    Engine::QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    ASSERT(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) == VK_SUCCESS);
}
void VulkanContext::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(swapChain.imageCount);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(swapChain.imageCount);

    ASSERT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) == VK_SUCCESS);
}
void VulkanContext::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(swapChain.imageCount, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.imageCount);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapChain.imageCount);

    ASSERT(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) == VK_SUCCESS);

    for (size_t i = 0; i < swapChain.imageCount; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(Engine::UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}
void VulkanContext::createCommandBuffers() {
    commandBuffers.resize(swapChain.imageCount);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    ASSERT(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) == VK_SUCCESS);
}
void VulkanContext::createSyncObjects() {
    imageAvailableSemaphores.resize(swapChain.imageCount);
    renderFinishedSemaphores.resize(swapChain.imageCount);
    inFlightFences.resize(swapChain.imageCount);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < swapChain.imageCount; i++) {
        ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VK_SUCCESS);
        ASSERT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VK_SUCCESS);
        ASSERT(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) == VK_SUCCESS);
    }
}
void VulkanContext::drawFrame() {
	uint32_t currentFrame = window.getCurrentFrame();
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

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
    updateUniformBuffer(currentFrame);

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
    VkSwapchainKHR swapChains[] = { swapChain.swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasFramebufferResized()) swapChain.recreate(*this);
    else ASSERT(result == VK_SUCCESS);

    window.resetCurrentFrame(swapChain.imageCount);
}
// called every frame - only these 2
void VulkanContext::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    ASSERT(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS);

    VkImageMemoryBarrier2 imageBarrierToAttachment{};
    imageBarrierToAttachment.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrierToAttachment.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    imageBarrierToAttachment.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageBarrierToAttachment.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    imageBarrierToAttachment.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageBarrierToAttachment.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrierToAttachment.image = swapChain.images[imageIndex];
    imageBarrierToAttachment.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    
    VkDependencyInfo dependencyInfoToAttachment{};
    dependencyInfoToAttachment.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfoToAttachment.imageMemoryBarrierCount = 1;
    dependencyInfoToAttachment.pImageMemoryBarriers = &imageBarrierToAttachment;
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfoToAttachment);

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = swapChain.imageViews[imageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = { {0, 0}, swapChain.extent };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipeline());

    VkViewport viewport{};
    viewport.width = static_cast<float>(swapChain.extent.width);
    viewport.height = static_cast<float>(swapChain.extent.height);
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);


    VkRect2D scissor{};
    scissor.extent = swapChain.extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    Engine::PushConstantModel pushConstant{};
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getLayout(), 0, 1, &descriptorSets[window.getCurrentFrame()], 0, nullptr);

 //   // Orbit parameters
 //   float orbitSpeedDegPerSec = 90.0f;   // degrees per second
 //   float orbitRadius = 3.0f;            // distance from terrain center
 //   glm::vec3 orbitAxis = glm::vec3(0.0f, 1.0f, 0.0f);

    //Build transform: rotate around terrainPos, then apply an offset (radius) and scale
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 Cube_scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 4.0f));

    // Final model matrix: rotation around terrain * offset from pivot * local scale
	mesh.bind(commandBuffer);
    pushConstant.model = rotation * Cube_scale;
    vkCmdPushConstants(commandBuffer, pipeline.getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Engine::PushConstantModel), &pushConstant);
    mesh.draw(commandBuffer);


    vkCmdEndRendering(commandBuffer);

    VkImageMemoryBarrier2 imageBarrierToPresent{};
    imageBarrierToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrierToPresent.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    imageBarrierToPresent.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    imageBarrierToPresent.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    imageBarrierToPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageBarrierToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imageBarrierToPresent.image = swapChain.images[imageIndex];
    imageBarrierToPresent.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo dependencyInfoToPresent{};
    dependencyInfoToPresent.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfoToPresent.imageMemoryBarrierCount = 1;
    dependencyInfoToPresent.pImageMemoryBarriers = &imageBarrierToPresent;
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfoToPresent);

	ASSERT(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS);
}
void VulkanContext::updateUniformBuffer(uint32_t currentImage) 
{
    Engine::UniformBufferObject ubo = camera.getCameraUBO();

    uniformBuffers[currentImage].write(device, &ubo, sizeof(ubo));
}

// --- Helper Methods ---
bool VulkanContext::isDeviceSuitable(VkPhysicalDevice device) {
    Engine::QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        Engine::SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &dynamicRenderingFeatures;
    vkGetPhysicalDeviceFeatures2(device, &features2);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && dynamicRenderingFeatures.dynamicRendering;
}
bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}
Engine::QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device) {
    Engine::QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
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
Engine::SwapChainSupportDetails VulkanContext::querySwapChainSupport(VkPhysicalDevice device) {
    Engine::SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}
VkSurfaceFormatKHR VulkanContext::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}
VkPresentModeKHR VulkanContext::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D VulkanContext::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
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
std::vector<const char*> VulkanContext::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}
bool VulkanContext::checkValidationLayerSupport() {
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
    //calculate delta time
    static auto lastFrameTime = std::chrono::high_resolution_clock::now();
    auto currentFrameTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentFrameTime - lastFrameTime).count();
    lastFrameTime = currentFrameTime;
    float sprintSpeed = 5.0f;

    // keyboard input to move camera
    //branchless
    window.setShouldClose(inputHandler.isKeyPressed(GLFW_KEY_ESCAPE));
	deltaTime = deltaTime + deltaTime * (sprintSpeed - 1.0f) * inputHandler.isKeyPressed(GLFW_KEY_LEFT_SHIFT);  // speed up when shift is held
    float movespeed = 3.0f * deltaTime;

    camera.moveForward(movespeed * inputHandler.isKeyPressed(GLFW_KEY_W));
    camera.moveForward(-movespeed * inputHandler.isKeyPressed(GLFW_KEY_S));
    camera.moveRight(movespeed * inputHandler.isKeyPressed(GLFW_KEY_D));
    camera.moveRight(-movespeed * inputHandler.isKeyPressed(GLFW_KEY_A));
    camera.moveUp(movespeed * inputHandler.isKeyPressed(GLFW_KEY_SPACE));
    camera.moveUp(-movespeed * inputHandler.isKeyPressed(GLFW_KEY_LEFT_CONTROL));

    // branching
    //if (inputHandler.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) deltaTime *= sprintSpeed; // speed up when shift is held
    //float movespeed = 3.0f * deltaTime;
    //if (inputHandler.isKeyPressed(GLFW_KEY_W)) camera.moveForward(movespeed);
    //if (inputHandler.isKeyPressed(GLFW_KEY_S)) camera.moveForward(-movespeed);
    //if (inputHandler.isKeyPressed(GLFW_KEY_D)) camera.moveRight(movespeed);
    //if (inputHandler.isKeyPressed(GLFW_KEY_A)) camera.moveRight(-movespeed);
    //if (inputHandler.isKeyPressed(GLFW_KEY_SPACE)) camera.moveUp(movespeed);
    //if (inputHandler.isKeyPressed(GLFW_KEY_LEFT_CONTROL)) camera.moveUp(-movespeed);


    // Camera rotation with mouse
    double mouseX, mouseY;
    inputHandler.getMouseDelta(mouseX, mouseY);
    if (mouseX != 0.0 || mouseY != 0.0)
    {
        float sensitivity = 0.1f;
        camera.rotate(static_cast<float>(mouseX * sensitivity), static_cast<float>(-mouseY * sensitivity));
    }
}
void VulkanContext::cleanFences(std::vector<VkFence>& fences)
{
    for (VkFence& fence : fences) {
        if (fence != VK_NULL_HANDLE) {
            vkDestroyFence(device, fence, nullptr);
            fence = VK_NULL_HANDLE;
        }
    }
    fences.clear();
}