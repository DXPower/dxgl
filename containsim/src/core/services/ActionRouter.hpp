#pragma once

#include <common/ActionChain.hpp>
#include <services/interfaces/IActionReceiver.hpp>
#include <services/interfaces/IMouseTester.hpp>

#include <glm/vec2.hpp>

namespace services {
    class ActionRouter final : public ActionConsumer {
    public:
        ActionProducer game_action_receiver{};
        ActionProducer ui_action_receiver{};
        ActionProducer offscreen_action_receiver{};

    private:
        const IMouseTester* m_mouse_tester{};
        InputLayer m_click_start{};

    public:
        ActionRouter(const IMouseTester& mouse_tester);
        
        void Consume(Action&& action) override;
    };
}