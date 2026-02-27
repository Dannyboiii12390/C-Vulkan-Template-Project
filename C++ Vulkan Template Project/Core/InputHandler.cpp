#include "InputHandler.h"
#include "Window.h"
#include <iostream>

namespace Engine {

    // Initialize static member
    std::unordered_map<GLFWwindow*, InputHandler*> InputHandler::s_instances;

    InputHandler::InputHandler(Window& window) : m_window(&window), m_lastMouseX(0), m_lastMouseY(0) {
        GLFWwindow* const win = m_window->getGLFWwindow();
      
        // Register this instance in the static map
        s_instances[win] = this;

        // Set up GLFW callbacks
        glfwSetKeyCallback(win, keyCallback);
        glfwSetMouseButtonCallback(win, mouseButtonCallback);
        
        if(!CURSOR) glfwSetCursorPosCallback(win, cursorPosCallback);
        
        glfwSetScrollCallback(win, scrollCallback);

        // Initialize mouse position
        glfwGetCursorPos(win, &m_mouseX, &m_mouseY);
        m_lastMouseX = m_mouseX;
        m_lastMouseY = m_mouseY;
        
        // Enable cursor capture for reliable mouse movement
        if(CURSOR) glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    InputHandler::~InputHandler() {
		GLFWwindow* const win = m_window->getGLFWwindow();
        if (m_window && win) {
            s_instances.erase(win);
        }
    }
    void InputHandler::update() {
        // Update mouse deltas
        m_mouseDeltaX = m_mouseX - m_lastMouseX;
        m_mouseDeltaY = m_mouseY - m_lastMouseY;
        m_lastMouseX = m_mouseX;
        m_lastMouseY = m_mouseY;

        // Reset scroll values (they're one-time events)
        m_scrollX = 0.0;
        m_scrollY = 0.0;

        // Update key states
        for (auto& [key, state] : m_keyStates) {
            if (state == State::PRESSED) {
                state = State::HELD;
            }
            else if (state == State::RELEASED_THIS_FRAME) {
                state = State::RELEASED;
            }
        }

        // Update mouse button states
        for (auto& [button, state] : m_mouseButtonStates) {
            if (state == State::PRESSED) {
                state = State::HELD;
            }
            else if (state == State::RELEASED_THIS_FRAME) {
                state = State::RELEASED;
            }
        }
    }

	// Keyboard methods
    bool InputHandler::isKeyPressed(int key) const {
		//return glfwGetKey(m_window->getGLFWwindow(), key) == GLFW_PRESS;
        const auto it = m_keyStates.find(key);
        return it != m_keyStates.end() && (it->second == State::PRESSED || it->second == State::HELD);
    }
    bool InputHandler::isKeyHeld(int key) const {
        const auto it = m_keyStates.find(key);
        return it != m_keyStates.end() && (it->second == State::PRESSED || it->second == State::HELD);
    }
    bool InputHandler::isKeyReleased(int key) const {
        const auto it = m_keyStates.find(key);
        return it != m_keyStates.end() && it->second == State::RELEASED_THIS_FRAME;
    }
    InputHandler::State InputHandler::getKeyState(int key) const {
        const auto it = m_keyStates.find(key);
        return it != m_keyStates.end() ? it->second : State::RELEASED;
    }

	// Mouse methods
    bool InputHandler::isMouseButtonPressed(int button) const {
		return glfwGetMouseButton(m_window->getGLFWwindow(), button) == GLFW_PRESS;
        /*auto it = m_mouseButtonStates.find(button);
        return it != m_mouseButtonStates.end() && (it->second == State::PRESSED || it->second == State::HELD);*/
    }
    bool InputHandler::isMouseButtonHeld(int button) const {
        const auto it = m_mouseButtonStates.find(button);
        return it != m_mouseButtonStates.end() && (it->second == State::PRESSED || it->second == State::HELD);
    }
    bool InputHandler::isMouseButtonReleased(int button) const {
        const auto it = m_mouseButtonStates.find(button);
        return it != m_mouseButtonStates.end() && it->second == State::RELEASED_THIS_FRAME;
    }
    InputHandler::State InputHandler::getMouseButtonState(int button) const {
        const auto it = m_mouseButtonStates.find(button);
        return it != m_mouseButtonStates.end() ? it->second : State::RELEASED;
    }
    void InputHandler::getMouseDelta(double& outDeltaX, double& outDeltaY) const noexcept {
        outDeltaX = m_mouseDeltaX;
        outDeltaY = m_mouseDeltaY;
    }

	// Callback registration
    void InputHandler::registerKeyCallback(KeyCallback& callback) {
        m_keyCallbacks.push_back(std::move(callback));
    }
    void InputHandler::registerMouseButtonCallback(MouseButtonCallback& callback) {
        m_mouseButtonCallbacks.push_back(std::move(callback));
    }
    void InputHandler::registerMouseMoveCallback(MouseMoveCallback& callback) {
        m_mouseMoveCallbacks.push_back(std::move(callback));
    }
    void InputHandler::registerMouseScrollCallback(MouseScrollCallback& callback) {
        m_scrollCallbacks.push_back(std::move(callback));
    }

    // Static GLFW callback functions
    void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        const auto it = s_instances.find(window);
        if (it == s_instances.end()) return;

        InputHandler* const instance = it->second;

        if (action == GLFW_PRESS) {
            instance->m_keyStates[key] = State::PRESSED;
        }
        else if (action == GLFW_RELEASE) {
            instance->m_keyStates[key] = State::RELEASED_THIS_FRAME;
        }

        for (const auto& callback : instance->m_keyCallbacks) {
            callback(key, scancode, action, mods);
        }
    }
    void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        const auto it = s_instances.find(window);
        if (it == s_instances.end()) return;

        InputHandler* const instance = it->second;

        if (action == GLFW_PRESS) {
            instance->m_mouseButtonStates[button] = State::PRESSED;
        }
        else if (action == GLFW_RELEASE) {
            instance->m_mouseButtonStates[button] = State::RELEASED_THIS_FRAME;
        }

        for (const auto& callback : instance->m_mouseButtonCallbacks) {
            callback(button, action, mods);
        }
    }
    void InputHandler::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        const auto it = s_instances.find(window);
        if (it == s_instances.end()) return;

        InputHandler* const instance = it->second;
        instance->m_mouseX = xpos;
        instance->m_mouseY = ypos;

        for (const auto& callback : instance->m_mouseMoveCallbacks) {
            callback(xpos, ypos);
        }
    }
    void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        const auto it = s_instances.find(window);
        if (it == s_instances.end()) return;

        InputHandler* const instance = it->second;
        instance->m_scrollX += xoffset;
        instance->m_scrollY += yoffset;

        for (const auto& callback : instance->m_scrollCallbacks) {
            callback(xoffset, yoffset);
        }
    }
}