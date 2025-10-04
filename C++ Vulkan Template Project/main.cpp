
#include "VulkanObjects/VulkanApplication.h"
#include <iostream>

int main() {
    // Cube vertices
    std::vector<Engine::Vertex> cube1 = {
        {{-0.5f,-0.5f,-0.5f},{1,0,0}}, {{0.5f,-0.5f,-0.5f},{0,1,0}},
        {{0.5f,0.5f,-0.5f},{0,0,1}}, {{-0.5f,0.5f,-0.5f},{1,1,0}},
        {{-0.5f,-0.5f,0.5f},{1,0,1}}, {{0.5f,-0.5f,0.5f},{0,1,1}},
        {{0.5f,0.5f,0.5f},{1,1,1}}, {{-0.5f,0.5f,0.5f},{0,0,0}}
    };

    std::vector<Engine::Vertex> cube2 = cube1;
    for (auto& v : cube2) v.pos.x += 1.0f; // offset second cube

    VulkanApplication app;
    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}