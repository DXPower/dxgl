#pragma once

#include <common/ActionChain.hpp>
#include <services/InputStateEvents.hpp>
#include <services/commands/BuildInputCommands.hpp>
#include <services/commands/CommandChains.hpp>
#include <services/commands/InputStateCommands.hpp>
#include <services/Logging.hpp>
#include <services/EventManager.hpp>
#include <dxfsm/dxfsm.hpp>

namespace services {
    class BuildInput;

    class InputState 
        // : public commands::CommandConsumer<commands::InputStateCommand>
        : public ActionConsumer {
    
        using StateId = services::InputStates;

        enum class EventId {
            ExitMode,
            EnterBuildMode,
            PauseGame,
            Action
        };

        using FSM_t = dxfsm::FSM<StateId, EventId>;
        using State_t = FSM_t::State_t;
        using Event_t = FSM_t::Event_t;
        FSM_t m_fsm{};
        EventManager* m_event_manager{};
        ActionProducer* m_action_forward{};

        logging::Logger m_logger = logging::CreateLogger("InputState");

    public:
        // commands::CommandProducer<commands::BuildInputCommand> build_input_cmds{};
        ActionProducer build_actions{};
        ActionProducer idle_actions{};

        InputState(EventManager& em, BuildInput& build_input);

        // void Consume(commands::InputStateCommandPtr&& cmd) override;
        void ProcessCommand(const commands::InputStateCommand& cmd);
        void Consume(Action&& action) override;

        void EnterBuildMode();
        void EnterStaffManagement();
        void EnterEconomyManagement();
        void Pause();
        void ExitMode();

    private:
        State_t StateIdle(FSM_t& fsm, StateId);
        State_t StatePauseMenu(FSM_t& fsm, StateId);
        State_t StateBuildActive(FSM_t& fsm, StateId); 
    };
}