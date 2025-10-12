
#include "VulkanContext.h"
#include <iostream>


/* todo: break into components
Device — Physical/logical device and queue setup

RenderPass — Render pass setup

Framebuffer — Framebuffers for each swapchain image

CommandPool/CommandBuffer — Command buffer management

ShaderModule — Shader loading and management

Texture — Texture image and sampler

DescriptorSet — Descriptor set layouts and updates

Synchronization — Fences, semaphores for frame sync

MemoryAllocator — Memory management abstraction

Camera — Camera matrices and controls

InputHandler — Input processing

DebugUtils — Validation and profiling helpers
*/

int main() {

    VulkanContext app;
    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}