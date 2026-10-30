#include <modules/input/RootTool.hpp>
#include <GLFW/glfw3.h>

using namespace input;

RootTool::RootTool() {
    ToolEvent::Builtin::InitIds(m_registry);

    // By default, if cancel isn't captured, exit to the next parent tool.
    On(ToolEvent::Builtin::Cancel, [this](InputTool&, const ToolEvent&, ToolEventControl&) {
        // If we're currently dragging, cancel the drag.
        auto drag_cancel_ev = m_drag_tracker.TryCancelDrag();
        
        if (drag_cancel_ev.has_value()) {
            GetActiveChild().Dispatch(*drag_cancel_ev);
            return;
        }

        // Otherwise, exit the current tool.
        if (HasChild()) {
            GetActiveChild().ExitToParent();
        }
    });

    Hotkeys().BindKey(GLFW_KEY_ESCAPE, ToolEvent::Builtin::Cancel);

    On(ToolEvent::Builtin::Click, std::bind_front(&RootTool::TryControlDrag, this));
}

void RootTool::RegisterTool(std::unique_ptr<InputTool> tool, int event_to_activate) {
    On(event_to_activate, [this, tool_ptr = tool.get()](InputTool&, const ToolEvent&, ToolEventControl&) {
        ReturnToSelf();
        EnterChild(*tool_ptr);
    });

    m_tools.push_back(std::move(tool));
}

void RootTool::UpdateMousePos(const MousePayload& payload) {
    auto ev = m_drag_tracker.OnMoveEvent(payload);

    if (ev.has_value()) {
        GetActiveChild().Dispatch(*ev);
    }
}

void RootTool::OnChildExit([[maybe_unused]] InputTool& child) {
    m_logger.info("RootTool is now active");
}

void RootTool::TryControlDrag(InputTool&, const ToolEvent& ev, ToolEventControl&) {
    auto drag_ev = m_drag_tracker.OnClickEvent(GetPayloadAs<MousePayload>(ev.payload));

    if (drag_ev.has_value()) {
        GetActiveChild().Dispatch(*drag_ev);
    }
}