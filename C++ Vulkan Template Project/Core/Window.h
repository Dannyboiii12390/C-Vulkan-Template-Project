#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    // Core window functions
    bool shouldClose() const;
    void pollEvents() const;

    // Accessors
    GLFWwindow* getGLFWwindow() const { return window; }
    VkExtent2D getExtent() const { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

    // Resize handling
    bool wasResized() const { return framebufferResized; }
    void resetResizeFlag() { framebufferResized = false; }

private:
    void initWindow();

    int width;
    int height;
    std::string title;
    GLFWwindow* window = nullptr;
    bool framebufferResized = false;

    // Static callback for GLFW
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};
