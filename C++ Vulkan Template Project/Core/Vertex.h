#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <vector>


//struct Vertex
//{
//    glm::vec3 pos;
//    glm::vec3 color;
//
//    // --- Vertex Definition ---
//    static VkVertexInputBindingDescription getBindingDescription();
//    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
//};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

