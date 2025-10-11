#pragma once
#include <GLFW/glfw3.h>
#include "../Debug Utils.h"
#include <vector>

namespace Engine 
{
	class Window 
	{
	private:
		int width;
		int height;
		const char* title;
		bool framebufferResized = false;
		GLFWwindow* window;
		uint32_t currentFrame = 0;


		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	public:

		Window(int w = 800, int h = 600, const char* t = "Default Title");
		~Window();

		const uint32_t getCurrentFrame() const { return currentFrame; }
		void resetCurrentFrame(size_t size) { currentFrame = (currentFrame + 1) % size; }
		GLFWwindow* getGLFWwindow() const { return window; }
		int getWidth() const { return width; }
		int getHeight() const { return height; }
		bool wasFramebufferResized() const { return framebufferResized; }
		void resetFramebufferResizedFlag() { framebufferResized = false; }
		bool shouldClose() const { return glfwWindowShouldClose(window); }
	

	};
}