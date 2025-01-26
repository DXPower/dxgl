#include <services/GameState.hpp>
#include <services/Logging.hpp>
#include <spdlog/spdlog.h>
#include <format>
#include <string_view>

#include <magic_enum/magic_enum.hpp>
#include <cofsm/CoFSM.hpp>

using namespace services;

using CoFSM::FSM;
using CoFSM::State;
using CoFSM::Event;

namespace {
    enum class StateType {
        NormalMode,
        BuildMode
    };

    enum class EventType {
        EnterBuildMode,
        ExitMode
    };

    constexpr std::string_view GetEventName(EventType type) {
        return magic_enum::enum_name(type);
    }

    bool EventMatches(Event& e, EventType type) {
        return e == GetEventName(type);
    }

    template<typename... T>
    requires (sizeof...(T) <= 1)
    Event MakeEvent(EventType type, T&&... arg) {
        Event e;
        e.construct(GetEventName(type), std::forward<T>(arg)...);
        return e;
    }
}

class GameState::Pimpl {
public:
    FSM fsm{"GameStateFSM"};
    logging::Logger logger{logging::CreateLogger("GameState")};

    Pimpl() {
        fsm << (NormalMode(fsm) = "NormalMode")
            << (BuildMode(fsm) = "BuildMode");

        using CoFSM::transition;

        fsm << transition("NormalMode", "BuildMode", GetEventName(EventType::EnterBuildMode))
            << transition("BuildMode", "NormalMode", GetEventName(EventType::ExitMode));

        fsm.logger = [this](const FSM&, const std::string& from_state, const Event& event, const std::string& to_state) {
            logger.info("Event {} sent from state {} to {}", event.nameAsString(), from_state, to_state);
        };

        fsm.start().setState("NormalMode");
    };

    State NormalMode(FSM& fsm) {
        Event event = co_await fsm.getEvent();

        while (true) {
            // if (EventMatches(event, EventType::ExitMode)) {
            //     event.construct(GetEventName(EventType::ExitMode))
            // } else {
            //     throw std::runtime_error(std::format("Unrecognized event {} received in BuildMode", event.nameAsString()));
            // }
            logger.info("In NormalMode!");

            event.destroy();
            event = co_await fsm.emitAndReceive(&event);
        }
    }

    State BuildMode(FSM& fsm) {
        Event event = co_await fsm.getEvent();

        while (true) {
            // if (EventMatches(event, EventType::ExitMode)) {
            //     event.construct(GetEventName(EventType::ExitMode))
            // } else {
            //     throw std::runtime_error(std::format("Unrecognized event {} received in BuildMode", event.nameAsString()));
            // }
            logger.info("In BuildMode!");
            event.destroy();
            event = co_await fsm.emitAndReceive(&event);
        }
    }
};

void GameState::PimplDeleter::operator()(Pimpl* ptr) const {
    delete ptr;
}

GameState::GameState() : m_pimpl(new Pimpl()) { }

void GameState::EnterBuildMode() {
    auto e = MakeEvent(EventType::EnterBuildMode);
    m_pimpl->fsm.sendEvent(&e);
}

void GameState::ExitMode() {
    auto e = MakeEvent(EventType::ExitMode);
    m_pimpl->fsm.sendEvent(&e);
}