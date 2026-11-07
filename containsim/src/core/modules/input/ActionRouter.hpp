#pragma once

#include <common/ActionChain.hpp>
#include <modules/input/IActionReceiver.hpp>
#include <modules/input/BasicMouseTester.hpp>
#include <modules/input/RootTool.hpp>
#include <modules/rendering/Camera.hpp>
#include <modules/core/TileGrid.hpp>
#include <common/MeceFsm.hpp>

#include <glm/vec2.hpp>

namespace input {
    class ActionRouter final : public ActionConsumer {
    public:
        ActionProducer global_action_producer{};

    private:
        BasicMouseTester m_mouse_tester;
        InputLayer m_click_start{};
        RootTool* m_root_tool{};
        const rendering::Camera* m_camera{};
        const core::TileGrid* m_tiles{};
        logging::Logger m_logger{logging::CreateLogger("ActionRouter")};

    public:
        ActionRouter(const dxgl::Window& window, RootTool& root_tool, const rendering::Camera& camera, const core::TileGrid& tiles);

        void Consume(Action&& action) override;
    };
}