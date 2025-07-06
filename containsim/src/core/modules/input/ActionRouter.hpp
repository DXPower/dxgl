#pragma once

#include <common/ActionChain.hpp>
#include <modules/input/IActionReceiver.hpp>
#include <modules/input/BasicMouseTester.hpp>

#include <glm/vec2.hpp>

namespace input {
    class ActionRouter final : public ActionConsumer {
    public:
        ActionProducer game_action_receiver{};
        ActionProducer offscreen_action_receiver{};

    private:
        BasicMouseTester m_mouse_tester;
        InputLayer m_click_start{};

    public:
        ActionRouter(const dxgl::Window& window);

        void Consume(Action&& action) override;
    };
}