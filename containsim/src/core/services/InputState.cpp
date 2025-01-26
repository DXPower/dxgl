#include <services/InputState.hpp>
#include <services/BuildInput.hpp>
#include <GLFW/glfw3.h>

using namespace services;

InputState::InputState(BuildInput& build_input) {
    StateIdle(m_fsm, StateId::IdleMode);
    StateBuildActive(m_fsm, StateId::BuildActive);
    m_fsm.SetCurrentState(StateId::IdleMode);

    m_fsm.AddTransition(StateId::IdleMode, EventId::EnterBuildMode, StateId::BuildActive);

    m_fsm.AddRemoteTransition(
        StateId::BuildActive,
        EventId::EnterBuildMode,
        build_input.GetFsm(),
        BuildInput::StateId::IdleMode,
        BuildInput::EventId::Entry);

    build_input.GetFsm().AddRemoteTransition(
        BuildInput::StateId::IdleMode,
        BuildInput::EventId::ExitMode,
        m_fsm,
        StateId::IdleMode,
        EventId::ExitMode
    );
}

auto InputState::StateIdle(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};
    while (true) {
        co_await fsm.EmitAndReceive(event);
        m_logger.debug("In StateIdle");

        if (event == EventId::Action) {
            auto& action = event.Get<Action>();
            const auto* press = std::get_if<KeyPress>(&action.data);

            if (press != nullptr) {
                if (press->IsDownKey(GLFW_KEY_B)) {
                    event = EventId::EnterBuildMode;
                    continue;
                } else if (press->IsDownKey(GLFW_KEY_ESCAPE)) {
                    event = EventId::PauseGame;
                    continue;
                }
            }

            // Forward uncaptured actions to the next layer
            idle_actions.Send(std::move(action));
        }

        event.Clear();
    }
}

auto InputState::StatePauseMenu(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};

    while (true) {
        co_await fsm.EmitAndReceive(event);
        m_logger.debug("In StatePauseMenu");

        if (event == EventId::Action) {
            const auto* press = std::get_if<KeyPress>(&event.Get<Action>().data);

            if (press->IsDownKey(GLFW_KEY_ESCAPE)) {
                event = EventId::ExitMode;
                continue;
            }
        }

        event.Clear();
    }
}

auto InputState::StateBuildActive(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};

    while (true) {
        co_await fsm.EmitAndReceive(event);
        m_logger.debug("In StateBuildActive");

        if (event == EventId::EnterBuildMode) {
            // Just forward the event to trigger the remote transition to the Build FSM
            continue;
        } else if (event == EventId::Action) {
            build_actions.Send(std::move(event.Get<Action>()));
        }

        event.Clear();
    }
}

void InputState::Consume(commands::InputStateCommandPtr&& cmd) {
    cmd->Execute(*this);
}

void InputState::Consume(Action&& action) {
    m_fsm.InsertEvent(EventId::Action, std::move(action));
}

void InputState::EnterBuildMode() {
    m_fsm.InsertEvent(EventId::EnterBuildMode);
}

void InputState::ExitMode() {
    m_fsm.InsertEvent(EventId::ExitMode);
}

void InputState::Pause() {
    m_fsm.InsertEvent(EventId::PauseGame);
}