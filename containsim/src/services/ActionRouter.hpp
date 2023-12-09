#pragma once

#include <services/interfaces/IActionReceiver.hpp>
#include <services/interfaces/IMouseTester.hpp>

#include <glm/vec2.hpp>

namespace services {
    class ActionRouter : public IActionReceiver {
        const IMouseTester* m_mouse_tester{};
        IActionReceiver* m_game_action_receiver{};
        IActionReceiver* m_ui_action_receiver{};

    public:
        ActionRouter(const IMouseTester& mouse_tester, IActionReceiver& game_actions, IActionReceiver& ui_actions);
        
        void PushAction(Action&& action) override;
    };
}