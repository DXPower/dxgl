#pragma once

#include <common/MeceFsm.hpp>
#include <common/Action.hpp>

namespace input {
    struct DragStartedData {
        MouseClick begin{};
    };

    struct DragData {
        MouseClick begin{};
        MouseClick end{};
    };

    class DragHelper {
        MeceSubFsm* m_sub{};
        int m_return_state{};
        int m_drag_state{};
        int m_drag_started{};
        int m_drag_completed{};
        int m_drag_canceled{};
        int m_action{};

    public:
        DragHelper(MeceSubFsm& sub, int return_state_id, std::string name);

        int GetDragStateId() const { return m_drag_state; }
        int GetDragStartedId() const { return m_drag_started; }
        int GetDragCompletedId() const { return m_drag_completed; }
        int GetDragCanceledId() const { return m_drag_canceled; }

    private:
        MeceSubFsm::State StateDrag(MeceSubFsm::FSM& fsm, int) const;
    };
}