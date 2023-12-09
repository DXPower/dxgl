#include <services/ActionRouter.hpp>

#include <dxtl/overloaded.hpp>

using namespace services;

ActionRouter::ActionRouter(const IMouseTester& mouse_tester, IActionReceiver& game_actions, IActionReceiver& ui_actions)
    : m_mouse_tester(&mouse_tester),
      m_game_action_receiver(&game_actions),
      m_ui_action_receiver(&ui_actions)
{ }

void ActionRouter::PushAction(Action&& action) {
    InputLayer dest{};

    std::visit([&]<typename T>(const T& a) {
        if constexpr (std::is_same_v<T, MouseClick>) {
            dest = m_mouse_tester->TestMouse(a.pos);
        } else if constexpr (std::is_same_v<T, MouseMove>) {
            dest = m_mouse_tester->TestMouse(a.to);
        } else {
            dest = InputLayer::Game;
        }
    }, action.data);

    switch (dest) {
        case InputLayer::Game:
            m_game_action_receiver->PushAction(std::move(action));
            break;
        case InputLayer::Ui:
            m_ui_action_receiver->PushAction(std::move(action));
            break;
    }
}