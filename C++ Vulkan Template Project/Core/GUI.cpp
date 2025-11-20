
#include "GUI.h"
#include <array>
#include <cstdio>

bool GUI::create(GLFWwindow* window,
    VkInstance instance,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    uint32_t queueFamily,
    VkQueue queue,
    VkPipelineCache pipelineCache,
    VkRenderPass renderPass,
    uint32_t imageCount,
    VkCommandPool commandPool,
    VkAllocationCallbacks* allocator)
{
    if (m_initialized) return true;

    m_allocator = allocator;
    m_device = device;

    // Basic ImGui context + GLFW init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // Create descriptor pool for ImGui
    std::array<VkDescriptorPoolSize, 11> pool_sizes = {
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * static_cast<uint32_t>(pool_sizes.size());
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();

    VkResult err = vkCreateDescriptorPool(device, &pool_info, allocator, &m_descriptorPool);
    if (err != VK_SUCCESS) {
        LOG("Failed to create ImGui descriptor pool (VkResult %d)", err);
        return false;
    }

    // Setup ImGui Vulkan init info
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    init_info.QueueFamily = queueFamily;
    init_info.Queue = queue;
    init_info.PipelineCache = pipelineCache;
    init_info.DescriptorPool = m_descriptorPool;
    init_info.PipelineInfoMain.RenderPass = renderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.MinImageCount = imageCount;
    init_info.ImageCount = imageCount;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = allocator;
    init_info.CheckVkResultFn = [](VkResult result) {
        if (result != VK_SUCCESS) {
            std::fprintf(stderr, "ImGui Vulkan error: VkResult = %d\n", result);
        }
        };

    // ImGui_ImplVulkan_Init now handles font uploading automatically
    if (!ImGui_ImplVulkan_Init(&init_info)) {
        LOG("Failed to initialize ImGui Vulkan backend");
        return false;
    }

    // Remove all the manual font upload code - it's no longer needed!
    // The following code should be deleted:
    // - VkCommandBufferAllocateInfo alloc_info
    // - vkAllocateCommandBuffers
    // - vkBeginCommandBuffer
    // - ImGui_ImplVulkan_CreateFontsTexture (doesn't exist)
    // - vkEndCommandBuffer
    // - vkQueueSubmit
    // - vkDeviceWaitIdle
    // - ImGui_ImplVulkan_DestroyFontUploadObjects
    // - vkFreeCommandBuffers

    m_initialized = true;
    return true;
}

void GUI::newFrame()
{
    if (!m_initialized) return;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::render(VkCommandBuffer commandBuffer)
{
    if (!m_initialized) return;
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (draw_data)
        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
}

void GUI::shutdown()
{
    if (!m_initialized) return;

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_descriptorPool != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(m_device, m_descriptorPool, m_allocator);
        m_descriptorPool = VK_NULL_HANDLE;
    }

    m_initialized = false;
}