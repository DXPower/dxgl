#pragma once

#include <common/ActionChain.hpp>
#include <services/InputStateEvents.hpp>
#include <services/commands/BuildInputCommands.hpp>
#include <services/commands/CommandChains.hpp>
#include <services/commands/InputStateCommands.hpp>
#include <services/commands/RoomInputCommands.hpp>
#include <services/Logging.hpp>
#include <modules/core/EventManager.hpp>
#include <services/RoomInput.hpp>
#include <dxfsm/dxfsm.hpp>

namespace services {
    class BuildInput;

    class InputState 
        : public ActionConsumer {
    
        using StateId = services::InputStates;

        enum class EventId {
            ExitMode,
            EnterBuildMode,
            EnterRoomMode,
            PauseGame,
            Action
        };

        using FSM_t = dxfsm::FSM<StateId, EventId>;
        using State_t = FSM_t::State_t;
        using Event_t = FSM_t::Event_t;
        FSM_t m_fsm{};
        core::EventManager* m_event_manager{};
        ActionProducer* m_action_forward{};

        logging::Logger m_logger = logging::CreateLogger("InputState");

    public:
        ActionProducer build_actions{};
        ActionProducer room_actions{};
        ActionProducer idle_actions{};

        InputState(core::EventManager& em, BuildInput& build_input, RoomInput& room_input);

        void ProcessCommand(const commands::InputStateCommand& cmd);
        void Consume(Action&& action) override;

        void EnterBuildMode();
        void EnterRoomMode();
        void EnterStaffManagement();
        void EnterEconomyManagement();
        void Pause();
        void ExitMode();

    private:
        State_t StateIdle(FSM_t& fsm, StateId);
        State_t StatePauseMenu(FSM_t& fsm, StateId);
        State_t StateBuildActive(FSM_t& fsm, StateId); 
        State_t StateRoomActive(FSM_t& fsm, StateId);
    };
}