#pragma once

#include <common/ActionChain.hpp>
#include <common/InputStateEvents.hpp>
#include <services/commands/BuildInputCommands.hpp>
#include <services/commands/CommandChains.hpp>
#include <services/commands/InputStateCommands.hpp>
#include <services/commands/RoomInputCommands.hpp>
#include <services/Logging.hpp>
#include <modules/application/EventManager.hpp>
#include <modules/input/RoomInput.hpp>
#include <dxfsm/dxfsm.hpp>

namespace input {
    class BuildInput;

    class InputState 
        : public ActionConsumer {
    
        using StateId = InputStates;

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
        application::EventManager* m_event_manager{};
        ActionProducer* m_action_forward{};

        services::logging::Logger m_logger = services::logging::CreateLogger("InputState");

    public:
        ActionProducer build_actions{};
        ActionProducer room_actions{};
        ActionProducer idle_actions{};

        InputState(application::EventManager& em, BuildInput& build_input, RoomInput& room_input);

        void ProcessCommand(const services::commands::InputStateCommand& cmd);
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