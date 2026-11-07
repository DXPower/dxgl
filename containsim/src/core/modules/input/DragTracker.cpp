#include <modules/input/DragTracker.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace input;

std::optional<ToolEvent> DragTracker::OnClickEvent(const MousePayload& click) {
    std::optional<ToolEvent> result{};

    switch (m_state) {
    case State::Idle:
        // Begin a drag if we are clicking down
        if (click.dir == ButtonDir::Down) {
            m_click_begin = click;
            m_state = State::Armed;

            m_logger.debug("Dragging armed (button {}, pos {})", click.button, glm::to_string(click.screen_pos));
        }
        break;
    case State::Armed:
        // If we are clicking up with the same button, cancel the drag.
        if (click.dir == ButtonDir::Up && click.button == m_click_begin.button) {
            // We would be dragging if we moved past the threshold.
            // No drag was started, so don't send a cancel event.
            // But we do send a tap event.
            ToolEvent ev{};
            ev.id = ToolEvent::Builtin::Tap;
            ev.payload = click;
            result = ev;

            m_logger.debug("Tap (button {}, pos {})", click.button, glm::to_string(click.screen_pos));
            
            ReturnToIdle();
        }
        break;
    case State::Dragging:
        // If we are clicking up with the same button, send out the drag
        if (click.dir == ButtonDir::Up && click.button == m_click_begin.button) {
            ToolEvent ev{};
            ev.id = ToolEvent::Builtin::DragComplete;
            ev.payload = DragPayload{
                .begin = m_click_begin,
                .end = click
            };
            result = ev;

            m_logger.debug("Dragging completed (button {}, start {}, end {})", click.button, glm::to_string(m_click_begin.screen_pos), glm::to_string(click.screen_pos));
            
            ReturnToIdle();
        }
        break;
    }

    return result;
}

std::optional<ToolEvent> DragTracker::OnMoveEvent(const MousePayload& move) {
    switch (m_state) {
    case State::Idle: break; // Do nothing
    case State::Armed: {
        // Check for breaking the threshold
        if (glm::length2(move.screen_pos - m_click_begin.screen_pos) > m_drag_threshold_2) {
            m_state = State::Dragging;

            ToolEvent ev{};
            ev.id = ToolEvent::Builtin::DragStart;
            ev.payload = DragPayload{
                .begin = m_click_begin,
                .end = move
            };

            m_logger.debug("Dragging started (button {}, start {}, current {})", m_click_begin.button, glm::to_string(m_click_begin.screen_pos), glm::to_string(move.screen_pos));
            return ev;
        }
        break;
    }
    case State::Dragging: {
        ToolEvent ev{};
        ev.id = ToolEvent::Builtin::DragUpdate;
        ev.payload = DragPayload{
            .begin = m_click_begin,
            .end = move
        };
        return ev;
    }
    }
    return {};
}

std::optional<ToolEvent> DragTracker::TryCancelDrag() {
    if (m_state == State::Dragging) {
        ToolEvent ev{};
        ev.id = ToolEvent::Builtin::DragCancel;

        m_logger.debug("Dragging cancelled (button {}, start {})", m_click_begin.button, glm::to_string(m_click_begin.screen_pos));
        
        ReturnToIdle();
        
        return ev;
    } else if (m_state == State::Armed) {
        // No drag was started, just return to idle.
        ReturnToIdle();
    }

    return {};
}

void DragTracker::ReturnToIdle() {
    m_click_begin = {};
    m_state = State::Idle;
}