#include <modules/input/DragHelper.hpp>
#include <common/Action.hpp>
#include <GLFW/glfw3.h>

using namespace input;

DragHelper::DragHelper(dxfsm::FSM<int, int>& fsm, IdRegistry& states, IdRegistry& events, int return_state_id, std::string name) {
    m_return_state = return_state_id;

    m_drag_state = states.AddId(std::move(name));

    m_drag_started = events.GetOrAddId("DragStarted");
    m_drag_completed = events.GetOrAddId("DragCompleted");
    m_drag_canceled = events.GetOrAddId("DragCanceled");
    m_action = events.GetOrAddId("Action");

    StateDrag(fsm, m_drag_state);

    fsm.AddTransition(m_return_state, m_drag_started, m_drag_state);
    fsm.AddTransition(m_drag_state, m_drag_completed, m_return_state);
    fsm.AddTransition(m_drag_state, m_drag_canceled, m_return_state);
}

dxfsm::State<int> DragHelper::StateDrag(dxfsm::FSM<int, int>& fsm, int) const {
    dxfsm::Event<int> ev{};
    DragStartedData drag_data{};

    while (true) {
        co_await fsm.EmitAndReceive(ev);

        if (ev == m_drag_started) {
            drag_data = ev.Get<DragStartedData>();
        } else if (ev == m_action) {
            const auto& action = ev.Get<Action>();
            const auto* click = std::get_if<MouseClick>(&action.data);
            const auto* press = std::get_if<KeyPress>(&action.data);

            if (click != nullptr) {
                if (click->button != drag_data.begin.button) {
                    // Different mouse button pressed, cancel the drag
                    ev = m_drag_canceled;
                    continue;
                }

                assert(click->dir == ButtonDir::Up);

                DragData finished_data{};
                finished_data.begin = drag_data.begin;
                finished_data.end = *click;

                ev.Store(m_drag_completed, finished_data);
                continue;
            } else if (press != nullptr) {
                if (press->IsDownKey(GLFW_KEY_ESCAPE)) {
                    ev = m_drag_canceled;
                    continue;
                }
            }
        }

        ev.Clear();
    }
}