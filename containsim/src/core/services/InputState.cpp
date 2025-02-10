#include <services/InputState.hpp>
#include <services/BuildInput.hpp>
#include <services/ui/PanelCommands.hpp>
#include <GLFW/glfw3.h>

using namespace services;

InputState::InputState(EventManager& em, BuildInput& build_input) : m_event_manager(&em) {
    StateIdle(m_fsm, StateId::IdleMode);
    StateBuildActive(m_fsm, StateId::BuildActive);
    m_fsm.SetCurrentState(StateId::IdleMode);

    m_fsm.AddTransition(StateId::IdleMode, EventId::EnterBuildMode, StateId::BuildActive);
    m_fsm.AddTransition(StateId::BuildActive, EventId::ExitMode, StateId::IdleMode);

    build_input.GetFsm().AddRemoteTransition(
        BuildInput::StateId::IdleMode,
        BuildInput::EventId::ExitMode,
        m_fsm,
        StateId::IdleMode,
        EventId::ExitMode
    );

    em.RegisterSignal<commands::InputStateCommand>()
        .signal.connect<&InputState::ProcessCommand>(this);
}

auto InputState::StateIdle(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};
    while (true) {
        co_await fsm.EmitAndReceive(event);
        m_logger.debug("In StateIdle");

        if (event == EventId::Action) {
            auto& action = event.Get<Action*>();
            const auto* press = std::get_if<KeyPress>(&action->data);

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
            m_action_forward = &idle_actions;
        } else if (event == EventId::ExitMode) {
            // Just got here from another state
            m_event_manager->GetSignal<InputStateChanged>()
                .signal.fire(InputStateChanged{InputStates::IdleMode});
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
            const auto* press = std::get_if<KeyPress>(&event.Get<Action*>()->data);

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
        auto leaving = co_await fsm.EmitAndReceiveResettable(event);

        // If we're leaving this state, hide the build panel
        if (leaving) {
            // ui::HidePanel cmd{};
            // cmd.name = "build-panel";
            // m_event_manager->GetSignal<ui::PanelCommand>()
            //     .signal.fire(cmd);

            continue;
        }

        if (event == EventId::EnterBuildMode) {
            // Just got here from another mode, show the build panel
            // ui::ShowPanel cmd{};
            // cmd.name = "build-panel";
            // m_event_manager->GetSignal<ui::PanelCommand>()
            //     .signal.fire(cmd);

            m_event_manager->GetSignal<InputStateChanged>()
                .signal.fire(InputStateChanged{InputStates::BuildActive});
        } else if (event == EventId::Action) {
            m_action_forward = &build_actions;
        }

        event.Clear();
    }
}

void InputState::ProcessCommand(const commands::InputStateCommand& cmd) {
    cmd.Execute(*this);
}

void InputState::Consume(Action&& action) {
    m_fsm.InsertEvent(EventId::Action, &action);

    if (m_action_forward != nullptr) {
        m_action_forward->Send(std::move(action));
        m_action_forward = nullptr;
    }
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