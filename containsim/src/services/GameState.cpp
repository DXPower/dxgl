#include <services/GameState.hpp>
#include <services/Logging.hpp>
#include <spdlog/spdlog.h>
#include <format>
#include <string_view>

#include <magic_enum.hpp>
#include <cofsm/CoFSM.hpp>

using namespace services;

using CoFSM::FSM;
using CoFSM::State;
using CoFSM::Event;

namespace {
    enum class StateType {
        NormalPerspective,
        BuildPerspective
    };

    enum class EventType {
        EnterBuildPerspective,
        ExitPerspective
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
        fsm << (NormalPerspective(fsm) = "NormalPerspective")
            << (BuildPerspective(fsm) = "BuildPerspective");

        using CoFSM::transition;

        fsm << transition("NormalPerspective", "BuildPerspective", GetEventName(EventType::EnterBuildPerspective))
            << transition("BuildPerspective", "NormalPerspective", GetEventName(EventType::ExitPerspective));

        fsm.logger = [this](const FSM&, const std::string& from_state, const Event& event, const std::string& to_state) {
            logger.info("Event {} sent from state {} to {}", event.nameAsString(), from_state, to_state);
        };

        fsm.start().setState("NormalPerspective");
    };

    State NormalPerspective(FSM& fsm) {
        Event event = co_await fsm.getEvent();

        while (true) {
            // if (EventMatches(event, EventType::ExitPerspective)) {
            //     event.construct(GetEventName(EventType::ExitPerspective))
            // } else {
            //     throw std::runtime_error(std::format("Unrecognized event {} received in BuildPerspective", event.nameAsString()));
            // }
            logger.info("In NormalPerspective!");

            event.destroy();
            event = co_await fsm.emitAndReceive(&event);
        }
    }

    State BuildPerspective(FSM& fsm) {
        Event event = co_await fsm.getEvent();

        while (true) {
            // if (EventMatches(event, EventType::ExitPerspective)) {
            //     event.construct(GetEventName(EventType::ExitPerspective))
            // } else {
            //     throw std::runtime_error(std::format("Unrecognized event {} received in BuildPerspective", event.nameAsString()));
            // }
            logger.info("In BuildPerspective!");
            event.destroy();
            event = co_await fsm.emitAndReceive(&event);
        }
    }
};

void GameState::PimplDeleter::operator()(Pimpl* ptr) const {
    delete ptr;
}

GameState::GameState() : m_pimpl(new Pimpl()) { }

void GameState::EnterBuildPerspective() {
    auto e = MakeEvent(EventType::EnterBuildPerspective);
    m_pimpl->fsm.sendEvent(&e);
}

void GameState::ExitPerspective() {
    auto e = MakeEvent(EventType::ExitPerspective);
    m_pimpl->fsm.sendEvent(&e);
}