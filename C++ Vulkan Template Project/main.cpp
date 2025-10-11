
#include "VulkanContext.h"
#include <iostream>

//todo
//swapchain
//command pool?

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