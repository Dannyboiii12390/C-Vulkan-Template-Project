#include "Window.h"

namespace Engine {
	Window::Window(int w, int h, const char* t) : width(w), height(h), title(t)
	{
		ASSERT(glfwInit()); // create glfw instance

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		ASSERT(window); // create window

		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

	}
	Window::~Window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
		app->width = width;
		app->height = height;
	}
}