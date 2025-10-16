
#include "VulkanContext.h"
#include <iostream>

/* todo: break into components
Device — Physical/logical device and queue setup

RenderPass — Render pass setup

CommandPool/CommandBuffer — Command buffer management

ShaderModule — Shader loading and management

Texture — Texture image and sampler

Synchronization — Fences, semaphores for frame sync
*/

int main() {

    VulkanContext app;
    try 
    {
        while (!app.getWindow().shouldClose()) {
            glfwPollEvents();
			app.getInputHandler().update();
            app.drawFrame();
        }

        vkDeviceWaitIdle(app.getDevice());
        app.cleanup();

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//class Shader { //shader should have own pipeline , layout, and descriptor sets
//public:
//    VkPipeline pipeline;
//    VkPipelineLayout pipelineLayout;
//
//    VkDescriptorSetLayout descriptorSetLayout;
//    VkDescriptorPool descriptorPool;
//
//    // Maybe store multiple sets for double/triple buffering
//    std::vector<VkDescriptorSet> descriptorSets;
//
//    void createDescriptorSetLayout();
//    void allocateDescriptorSets();
//    void updateDescriptorSets(...); // pass buffers/images here
//
//    void bind(VkCommandBuffer cmdBuffer, VkPipelineBindPoint bindPoint);
//};

//Suggested Abstraction Layers
//Shader class : Owns layout + pipeline + descriptor sets.
//Material class (optional) : Owns per - instance resource data(e.g., per - object uniform buffer).
//Renderer or RenderPass : Manages command buffers, submits shaders and binds resources.