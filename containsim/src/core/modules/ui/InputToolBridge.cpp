#include <modules/ui/InputToolBridge.hpp>

using namespace ui;

InputToolBridge::InputToolBridge(application::EventManager& em, input::RootTool& tool)
    : m_tool(&tool) {

    em.GetOrRegisterSignal<ui_events::ElementEvent>()
        .signal.connect<&InputToolBridge::ProcessElementEvent>(this);
}

void InputToolBridge::ProcessElementEvent(const ui_events::ElementEvent& event) {
    if (event.args->size() == 0)
        return;

    const auto id = m_tool->Registry().GetId((*event.args)[0]);

    if (!id.has_value()) {
        m_logger.warn("InputToolBridge: Unknown event id: {}", (*event.args)[0]);
        return;
    }

    input::ToolEvent tool_event{
        .id = *id,
        .payload = input::UiPayload{
            .args = event.args
        }
    };

    const bool handled = m_tool->Dispatch(tool_event);

    if (!handled) {
        m_logger.warn("InputToolBridge: Event not handled: {}", (*event.args)[0]);
    }
}