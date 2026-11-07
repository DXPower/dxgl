#pragma once

#include <common/Logging.hpp>
#include <modules/input/ToolEvent.hpp>
#include <optional>

namespace input {

class DragTracker {
public:
    enum class State : uint8_t {
        Idle, Armed, Dragging
    };

private:
    static constexpr float m_drag_threshold_2 = 4.0f * 4.0f; // 4 pixels squared

    MousePayload m_click_begin{};
    State m_state{};

    logging::Logger m_logger{logging::CreateLogger("DragTracker")};
    
public:
    std::optional<ToolEvent> OnClickEvent(const MousePayload& click);
    std::optional<ToolEvent> OnMoveEvent(const MousePayload& move);
    std::optional<ToolEvent> TryCancelDrag();
    
    State GetState() const { return m_state; }

private:
    void ReturnToIdle();
};

}