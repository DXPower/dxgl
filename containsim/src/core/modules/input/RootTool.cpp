#include <modules/input/RootTool.hpp>
#include <GLFW/glfw3.h>

using namespace input;

RootTool::RootTool() {
    ToolEvent::Builtin::InitIds(m_registry);

    // By default, if cancel isn't captured, exit to the next parent tool.
    On(ToolEvent::Builtin::Cancel, [this](InputTool&, const ToolEvent&, ToolEventControl&) {
        if (HasChild()) {
            GetActiveChild().ExitToParent();
        }
    });

    Hotkeys().BindKey(GLFW_KEY_ESCAPE, ToolEvent::Builtin::Cancel);
}

void RootTool::RegisterTool(std::unique_ptr<InputTool> tool, int event_to_activate) {
    On(event_to_activate, [this, tool_ptr = tool.get()](InputTool&, const ToolEvent&, ToolEventControl&) {
        EnterChild(*tool_ptr);
    }, {
        .only_when_active = true
    });

    m_tools.push_back(std::move(tool));
}

void RootTool::OnChildExit([[maybe_unused]] InputTool& child) {
    m_logger.info("RootTool is now active");
}