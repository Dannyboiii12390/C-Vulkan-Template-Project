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
	void Window::setShouldClose(bool value) const {
		glfwSetWindowShouldClose(window, value);
	}

	void Window::framebufferResizeCallback(GLFWwindow* pWindow, int pWidth, int pHeight)
	{
		auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));
		app->framebufferResized = true;
		app->width = pWidth;
		app->height = pHeight;
	}
}