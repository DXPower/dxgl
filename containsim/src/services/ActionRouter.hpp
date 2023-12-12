#pragma once

#include <services/interfaces/IActionReceiver.hpp>
#include <services/interfaces/IMouseTester.hpp>

#include <glm/vec2.hpp>

namespace services {
    class ActionRouter : public IActionReceiver {
        const IMouseTester* m_mouse_tester{};
        IActionReceiver* m_game_action_receiver{};
        IActionReceiver* m_ui_action_receiver{};
        IActionReceiver* m_offscreen_action_receiver{};

        InputLayer m_click_start{};
    public:
        ActionRouter(
            const IMouseTester& mouse_tester,
            IActionReceiver& game_actions,
            IActionReceiver& ui_actions,
            IActionReceiver& offscreen_actions
        );
        
        void PushAction(Action&& action) override;
    };
}