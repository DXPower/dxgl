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

        glfwSetKeyCallback(glfw_window, &KeyCallback);
        glfwSetCharCallback(glfw_window, &TextCallback);
        glfwSetMouseButtonCallback(glfw_window, &CursorClickCallback);
        glfwSetCursorPosCallback(glfw_window, &CursorMoveCallback);
        glfwSetScrollCallback(glfw_window, &ScrollCallback);
    }

    Pimpl(const Pimpl&) = delete;
    Pimpl(Pimpl&&) = delete;
    Pimpl& operator=(const Pimpl&) = delete;
    Pimpl& operator=(Pimpl&&) = delete;

    ~Pimpl() {
        window_mappings.erase(glfw_window);
    }

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto pit = window_mappings.find(window);

        if (pit == window_mappings.end() || !pit->second->on_action)
            return;

        pit->second->on_action(Action{
            .data = KeyPress{
                .dir = static_cast<ButtonDir>(action),
                .key = key,
                .scancode = scancode,
                .mods = mods
            }
        });
    }

    static void TextCallback(GLFWwindow* window, unsigned int codepoint) {
        auto pit = window_mappings.find(window);

        if (pit == window_mappings.end() || !pit->second->on_action)
            return;

        pit->second->on_action(Action{
            .data = TextInput{
                .codepoint = codepoint
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

    static void ScrollCallback(GLFWwindow* glfw_window, double x, double y) {
        auto pit = window_mappings.find(glfw_window);

        if (pit == window_mappings.end() || !pit->second->on_action)
            return;

        glm::dvec2 mouse_pos{};
        glfwGetCursorPos(glfw_window, &mouse_pos.x, &mouse_pos.y);

        pit->second->on_action(Action{
            .data = ScrollInput{
                .pos = mouse_pos,
                .amount = {x, y}
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