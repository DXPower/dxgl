#pragma once

#include <common/ActionChain.hpp>
#include <modules/input/IActionReceiver.hpp>
#include <modules/input/BasicMouseTester.hpp>
#include <common/MeceFsm.hpp>

#include <glm/vec2.hpp>

namespace input {
    class ActionRouter final : public ActionConsumer {
    public:
        ActionProducer global_action_producer{};

    private:
        BasicMouseTester m_mouse_tester;
        InputLayer m_click_start{};
        MeceFsm* m_actions_out{};
        logging::Logger m_logger{logging::CreateLogger("ActionRouter")};

    public:
        ActionRouter(const dxgl::Window& window, MeceFsm& actions_out);

        void Consume(Action&& action) override;
    };
}