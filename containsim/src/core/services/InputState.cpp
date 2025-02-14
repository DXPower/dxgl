#include <services/InputState.hpp>
#include <services/BuildInput.hpp>
#include <services/ui/PanelCommands.hpp>
#include <services/commands/BuildInputCommands.hpp>
#include <GLFW/glfw3.h>
#include <magic_enum/magic_enum.hpp>

using namespace services;

InputState::InputState(EventManager& em, BuildInput& build_input) : m_event_manager(&em) {
    m_logger.set_level(spdlog::level::debug);

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

    m_fsm.SetTransitionObserver([this](const FSM_t&, std::optional<State_t> from, State_t to, const Event_t& ev) {
        std::string ev_str = !ev.Empty() ? std::format(" Event: {}", magic_enum::enum_name(ev.GetId())) : "";
        
        if (!from.has_value()) {
            m_logger.info("Transitioning to {}.{}", magic_enum::enum_name(to.Id()), ev_str);
        } else {
            m_logger.info(
                "Transitioning from {} to {}.{}",
                magic_enum::enum_name(from->Id()),
                magic_enum::enum_name(to.Id()),
                ev_str
            );
        }

        m_event_manager->GetSignal<InputStateChanged>()
            .signal.fire(InputStateChanged{to.Id()});

        if (from.has_value() && from->Id() == StateId::BuildActive) {
            commands::ResetBuildInput cmd{};
            m_event_manager->GetSignal<commands::BuildInputCommand>()
                .signal.fire(cmd);
        }
    });

    em.RegisterSignal<commands::InputStateCommand>()
        .signal.connect<&InputState::ProcessCommand>(this);
}

auto InputState::StateIdle(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};
    while (true) {
        co_await fsm.EmitAndReceive(event);

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
        }

        event.Clear();
    }
}

auto InputState::StatePauseMenu(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};

    while (true) {
        co_await fsm.EmitAndReceive(event);

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
        co_await fsm.EmitAndReceive(event);

        if (event == EventId::Action) {
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