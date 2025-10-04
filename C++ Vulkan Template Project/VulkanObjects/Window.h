#pragma once
#include <GLFW/glfw3.h>
#include "../Debug Utils.h"
#include "VulkanApplication.h"
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

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height) 
		{
			auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			app->framebufferResized = true;
			app->width = width;
			app->height = height;
		}
	public:
		Window() {};
		Window(int w, int h, const char* t) : width(w), height(h), title(t) 
		{
			ASSERT(glfwInit()); // create glfw instance

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			window = glfwCreateWindow(width, height, title, nullptr, nullptr);
			ASSERT(window); // create window
			
			glfwSetWindowUserPointer(window, this);
			glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

		}
		~Window() 
		{
			glfwDestroyWindow(window);
			glfwTerminate();
		}
		GLFWwindow* getGLFWwindow() const { return window; }
		int getWidth() const { return width; }
		int getHeight() const { return height; }
		bool wasFramebufferResized() const { return framebufferResized; }
		void resetFramebufferResizedFlag() { framebufferResized = false; }
		bool shouldClose() const { return glfwWindowShouldClose(window); }
	

	};
}