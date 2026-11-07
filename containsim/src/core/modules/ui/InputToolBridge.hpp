#pragma once

#include <modules/application/EventManager.hpp>
#include <modules/input/RootTool.hpp>
#include <modules/ui/UiEvents.hpp>
#include <common/Logging.hpp>

namespace ui {

class InputToolBridge {
    logging::Logger m_logger{logging::CreateLogger("InputToolBridge")};
    input::RootTool* m_tool{};
public:
    InputToolBridge(application::EventManager& em, input::RootTool& tool);

    void ProcessElementEvent(const ui_events::ElementEvent& event);
};

}