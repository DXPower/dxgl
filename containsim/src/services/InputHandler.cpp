#include "InputHandler.hpp"

#include <stdexcept>
#include <unordered_map>

#include <glfw/glfw3.h>

using namespace services;

class InputHandler::Pimpl {
public:
    inline static std::unordered_map<GLFWwindow*, Pimpl*> window_mappings{};

    GLFWwindow* glfw_window{};
    std::function<void(Action&&)> on_action{};
    glm::dvec2 last_mouse_pos{};

    Pimpl(const dxgl::Window& window) {
        if(window_mappings.contains(window.GetGlfwWindow()))
            throw std::runtime_error("Attempt to create more than one InputHandler for a single window");

        glfw_window = window.GetGlfwWindow();
        window_mappings.emplace(glfw_window, this);

        last_mouse_pos = window.GetMousePos();

        glfwSetMouseButtonCallback(glfw_window, &CursorClickCallback);
        glfwSetKeyCallback(glfw_window, &KeyCallback);
        glfwSetCursorPosCallback(glfw_window, &CursorMoveCallback);
    }

    ~Pimpl() {
        window_mappings.erase(glfw_window);
    }

    static void KeyCallback(GLFWwindow* window, int key, int scancode [[maybe_unused]], int action, int mods) {
        auto pit = window_mappings.find(window);

        if (pit == window_mappings.end() || !pit->second->on_action)
            return;

        pit->second->on_action(Action{
            .data = KeyPress{
                .dir = static_cast<ButtonDir>(action),
                .key = key,
                .mods = mods
            }
        });
    }

    static void CursorMoveCallback(GLFWwindow* window, double x, double y) {
        auto pit = window_mappings.find(window);

        if (pit == window_mappings.end())
            return;

        auto& pimpl = *pit->second;

        glm::dvec2 new_pos{x, y};
        glm::dvec2 old_pos = std::exchange(pimpl.last_mouse_pos, new_pos);

        if (pimpl.on_action) {
            pimpl.on_action(Action{
                .data = MouseMove{
                    .from = old_pos,
                    .to = new_pos
                }
            });
        }
    }

    static void CursorClickCallback(GLFWwindow* glfw_window, int button, int action, int mods) {
        auto pit = window_mappings.find(glfw_window);

        if (pit == window_mappings.end() || !pit->second->on_action)
            return;

        auto& pimpl = *pit->second;
        auto& window = dxgl::Window::GetWindowFromGlfw(glfw_window);

        pimpl.on_action(Action{
            .data = MouseClick{
                .dir = static_cast<ButtonDir>(action),
                .pos = window.GetMousePos(),
                .button = button,
                .mods = mods
            }
        });
    }
};

InputHandler::InputHandler(const dxgl::Window& window) {
    m_pimpl = std::make_unique<Pimpl>(window);
}

InputHandler::~InputHandler() = default;

void InputHandler::OnAction(std::function<void(Action&& action)> func) {
    m_pimpl->on_action = std::move(func);
}