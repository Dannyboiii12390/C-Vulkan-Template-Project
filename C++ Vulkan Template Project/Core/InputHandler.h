#pragma once

#include <unordered_map>
#include <functional>
#include <vector>
#include "Window.h"

constexpr int CURSOR = 1;

namespace Engine {

    class Window;

    class InputHandler final {
    public:
        // Key and mouse button states
        enum class State {
            RELEASED,
            PRESSED,
            HELD,
            RELEASED_THIS_FRAME
        };

        InputHandler() = default;
        explicit InputHandler(Window& window);

		InputHandler(const InputHandler&) = delete;
		InputHandler& operator=(const InputHandler&) = delete;
		InputHandler(InputHandler&&) = delete;
		InputHandler& operator=(InputHandler&&) = delete;

        ~InputHandler();

        // Must be called once per frame to update input states
        void update();

        // Keyboard methods
        bool isKeyPressed(int key) const;
		bool isMouseButtonPressed(int button) const;
        bool isKeyHeld(int key) const;
        bool isKeyReleased(int key) const;
        State getKeyState(int key) const;

        // Mouse button methods
        bool isMouseButtonHeld(int button) const;
        bool isMouseButtonReleased(int button) const;
        State getMouseButtonState(int button) const;

        // Mouse position and movement
        double getMouseX() const { return m_mouseX; }
        double getMouseY() const { return m_mouseY; }
        double getMouseDeltaX() const { return m_mouseDeltaX; }
        double getMouseDeltaY() const { return m_mouseDeltaY; }
        double getMouseScrollX() const { return m_scrollX; }
        double getMouseScrollY() const { return m_scrollY; }
        // Mouse delta since last update (pixels). Returns true if delta is valid.
        void getMouseDelta(double& outDeltaX, double& outDeltaY) const noexcept;

        // Event callbacks
        using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
        using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
        using MouseMoveCallback = std::function<void(double xpos, double ypos)>;
        using MouseScrollCallback = std::function<void(double xoffset, double yoffset)>;

        void registerKeyCallback(KeyCallback& callback);
        void registerMouseButtonCallback(MouseButtonCallback& callback);
        void registerMouseMoveCallback(MouseMoveCallback& callback);
        void registerMouseScrollCallback(MouseScrollCallback& callback);

    private:
        Window* m_window;

        // Group callbacks with their corresponding state containers to improve layout/cache locality
        std::vector<KeyCallback> m_keyCallbacks;
        std::unordered_map<int, State> m_keyStates;

        std::vector<MouseButtonCallback> m_mouseButtonCallbacks;
        std::unordered_map<int, State> m_mouseButtonStates;

        std::vector<MouseMoveCallback> m_mouseMoveCallbacks;
        std::vector<MouseScrollCallback> m_scrollCallbacks;

        // Mouse position tracking (grouped fixed-size doubles)
        double m_mouseX = 0.0, m_mouseY = 0.0;
        double m_lastMouseX = 0.0, m_lastMouseY = 0.0;
        double m_mouseDeltaX = 0.0, m_mouseDeltaY = 0.0;
        double m_scrollX = 0.0, m_scrollY = 0.0;

        // Static GLFW callback handlers
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

        // Map to lookup InputHandler instances by window
        static std::unordered_map<GLFWwindow*, InputHandler*> s_instances;
    };

}