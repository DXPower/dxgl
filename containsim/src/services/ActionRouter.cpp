#include <services/ActionRouter.hpp>

#include <dxtl/overloaded.hpp>
#include <magic_enum_containers.hpp>

using namespace services;

ActionRouter::ActionRouter(
    const IMouseTester& mouse_tester,
    IActionReceiver& game_actions,
    IActionReceiver& ui_actions,
    IActionReceiver& offscreen_actions
) : m_mouse_tester(&mouse_tester),
    m_game_action_receiver(&game_actions),
    m_ui_action_receiver(&ui_actions),
    m_offscreen_action_receiver(&offscreen_actions)
{ }

void ActionRouter::PushAction(Action&& action) {
    magic_enum::containers::array<InputLayer, bool> routes{};
    std::optional<glm::dvec2> mouse_pos{};

    bool update_click_start = false;

    std::visit([&]<typename T>(const T& a) {
        if constexpr (std::is_same_v<T, MouseClick>) {
            const MouseClick& click = a;

            mouse_pos = click.pos;

            if (click.dir == ButtonDir::Down) {
                update_click_start = true;
            } else if (click.dir == ButtonDir::Up) {
                // Notify where the click started that the click has stopped
                routes[m_click_start] = true;
            }
        } else if constexpr (std::is_same_v<T, MouseMove>) {
            mouse_pos = a.to;

            // Mouse movements should broadcast to everyone
            for (bool& route : routes) {
                route = true;
            }
        } else if constexpr (std::is_same_v<T, ScrollInput>) {
            mouse_pos = a.pos;
        } else if constexpr (std::is_same_v<T, KeyPress>) {
            // TODO: Check for keyboard focus in UI
            routes[InputLayer::Game] = true;
        }
    }, action.data);

    if (mouse_pos.has_value()) {
        const InputLayer route = m_mouse_tester->TestMouse(*mouse_pos);
        routes[route] = true;

        if (update_click_start)
            m_click_start = route;
    }

    if (routes[InputLayer::Game])
        m_game_action_receiver->PushAction(std::move(action));

    if (routes[InputLayer::Ui])
        m_ui_action_receiver->PushAction(std::move(action));

    if (routes[InputLayer::Offscreen])
        m_offscreen_action_receiver->PushAction(std::move(action));
}