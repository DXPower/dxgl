#include <modules/input/InputState.hpp>
#include <modules/input/BuildInput.hpp>
#include <GLFW/glfw3.h>
#include <magic_enum/magic_enum.hpp>

using namespace input;

InputState::InputState(application::EventManager& em, BuildInput& build_input, RoomInput& room_input)
    : EventCommandable(em)
    , m_event_manager(&em) {
    m_logger.set_level(spdlog::level::debug);

    StateIdle(m_fsm, StateId::IdleMode);
    StateBuildActive(m_fsm, StateId::BuildActive);
    StateRoomActive(m_fsm, StateId::RoomActive);

    m_fsm.SetCurrentState(StateId::IdleMode);

    m_fsm.AddTransition(StateId::IdleMode, EventId::EnterBuildMode, StateId::BuildActive);
    m_fsm.AddTransition(StateId::BuildActive, EventId::ExitMode, StateId::IdleMode);
    
    m_fsm.AddTransition(StateId::IdleMode, EventId::EnterRoomMode, StateId::RoomActive);
    m_fsm.AddTransition(StateId::RoomActive, EventId::ExitMode, StateId::IdleMode);

    m_fsm.AddTransition(StateId::BuildActive, EventId::EnterRoomMode, StateId::RoomActive);
    m_fsm.AddTransition(StateId::RoomActive, EventId::EnterBuildMode, StateId::BuildActive);

    build_input.GetFsm().AddRemoteTransition(
        BuildInput::StateId::IdleMode,
        BuildInput::EventId::ExitMode,
        m_fsm,
        StateId::IdleMode,
        EventId::ExitMode
    );

    room_input.GetFsm().AddRemoteTransition(
        RoomInput::StateId::IdleMode,
        RoomInput::EventId::ExitMode,
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
            m_event_manager->GetSignal<BuildInputCommand>()
                .signal.fire([](BuildInput& fsm) {
                        fsm.GetFsm().SetCurrentState(BuildInput::StateId::IdleMode);
                    });
        }
    });
}

auto InputState::StateIdle(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};
    m_action_forward = &idle_actions;
    
    while (true) {
        co_await fsm.EmitAndReceive(event);

        if (event == EventId::Action) {
            auto& action = event.Get<Action*>();
            const auto* press = std::get_if<KeyPress>(&action->data);

            if (press != nullptr) {
                if (press->IsDownKey(GLFW_KEY_B)) {
                    event = EventId::EnterBuildMode;
                    continue;
                } else if (press->IsDownKey(GLFW_KEY_R)) {
                    event = EventId::EnterRoomMode;
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

auto InputState::StateRoomActive(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};

    while (true) {
        co_await fsm.EmitAndReceive(event);

        if (event == EventId::Action) {
            m_action_forward = &room_actions;
        }

        event.Clear();
    }
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

void InputState::EnterRoomMode() {
    m_fsm.InsertEvent(EventId::EnterRoomMode);
}

void InputState::ExitMode() {
    m_fsm.InsertEvent(EventId::ExitMode);
}

void InputState::Pause() {
    m_fsm.InsertEvent(EventId::PauseGame);
}