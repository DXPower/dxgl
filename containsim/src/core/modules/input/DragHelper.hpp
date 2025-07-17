#pragma once

#include <common/IdRegistry.hpp>
#include <common/Action.hpp>

#include <dxfsm/dxfsm.hpp>

namespace input {
    struct DragStartedData {
        MouseClick begin{};
    };

    struct DragData {
        MouseClick begin{};
        MouseClick end{};
    };

    class DragHelper {
        int m_return_state{};
        int m_drag_state{};
        int m_drag_started{};
        int m_drag_completed{};
        int m_drag_canceled{};
        int m_action{};

    public:
        DragHelper(dxfsm::FSM<int, int>& fsm, IdRegistry& states, IdRegistry& events, int return_state_id, std::string name);

        int GetDragStateId() const { return m_drag_state; }
        int GetDragStartedId() const { return m_drag_started; }
        int GetDragCompletedId() const { return m_drag_completed; }
        int GetDragCanceledId() const { return m_drag_canceled; }

    private:
        dxfsm::State<int> StateDrag(dxfsm::FSM<int, int>& fsm, int) const;
    };
}