
#include "VulkanContext.h"
#include <iostream>
#include <fstream>
#include <chrono>

//C:\VulkanSDK\1.4.313.2\Include

/* todo: break into components
Device — Physical/logical device and queue setup

RenderPass — Render pass setup

CommandPool/CommandBuffer — Command buffer management

ShaderModule — Shader loading and management

Texture — Texture image and sampler

Synchronization — Fences, semaphores for frame sync

*/

int main() {

	std::fstream fpsFile("fps_log.txt", std::ios::out);
    
	float totalTime = 0.0f;
	int frameCount = 0.0f;

    VulkanContext app;
    try 
    {
        while (!app.getWindow().shouldClose()) {
			auto startTime = std::chrono::high_resolution_clock::now();
            glfwPollEvents();
			app.getInputHandler().update();
            app.drawFrame();
			auto endTime = std::chrono::high_resolution_clock::now();
			float elapsed = std::chrono::duration<float, std::milli>(endTime - startTime).count();
			totalTime += elapsed;
			frameCount++;
			float fps = 1000.0f / elapsed;
            fpsFile << fps << "\n";

        }
        fpsFile << std::endl;
		float averageFPS = frameCount / (totalTime / 1000.0f);
		std::cout << "Average FPS: " << averageFPS << std::endl;

        vkDeviceWaitIdle(app.getDevice());
        app.cleanup();

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


// Mesh -> Shader -> Pipeline -> Command Buffer -> Frame

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