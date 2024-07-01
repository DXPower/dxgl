#include <services/BuildMode.hpp>
#include <services/Logging.hpp>

#include <cofsm/CoFSM.hpp>
#include <magic_enum.hpp>
#include <glfw/glfw3.h>

#include <dxfsm/dxfsm.hpp>

using namespace services;
using namespace commands;

using CoFSM::FSM;
using CoFSM::State;
using CoFSM::Event;

namespace {
    enum class EventType {
        EnterBuildMode,
        EnterDeleteMode,
        ExitMode,
        SelectTile,
        Click,
        KeyPress
    };

    constexpr std::string_view GetEventName(EventType type) {
        return magic_enum::enum_name(type);
    }

    bool EventMatches(Event& e, EventType type) {
        return e == GetEventName(type);
    }

    // template<typename... T>
    // requires (sizeof...(T) <= 1)
    // Event MakeEvent(EventType type, T&&... arg) {
    //     Event e;
    //     e.construct(GetEventName(type), std::forward<T>(arg)...);
    //     return e;
    // }

    void ReplaceEvent(Event& e, EventType type) {
        e.construct(GetEventName(type));
    }

    template<typename T>
    void ReplaceEvent(Event& e, EventType type, T&& arg) {
        e.construct(GetEventName(type), std::forward<T>(arg));
    }
}

class BuildMode::Pimpl {
public:
    FSM fsm{"BuildModeFSM"};
    logging::Logger logger{logging::CreateLogger("BuildMode")};

    CommandProducer<StateCommand>* state_cmd_prod{};

    Pimpl(CommandProducer<StateCommand>& state_cmd_prod)
        : state_cmd_prod(&state_cmd_prod)
    {
        using namespace CoFSM;

        // fsm << 
        fsm << (StateIdle(fsm) = "Idle")
            << (StateBuildMode(fsm) = "BuildMode")
            << (StatePlaceMode(fsm) = "PlaceMode")
            << (StateDeleteMode(fsm) = "DeleteMode");

        fsm << transition("Idle", GetEventName(EventType::EnterBuildMode), "BuildMode")
            << transition("BuildMode", GetEventName(EventType::ExitMode), "Idle")
            << transition("DeleteMode", GetEventName(EventType::ExitMode), "BuildMode")
            << transition("BuildMode", GetEventName(EventType::EnterDeleteMode), "DeleteMode")
            << transition("DeleteMode", GetEventName(EventType::EnterBuildMode), "BuildMode")
            << transition("BuildMode", GetEventName(EventType::SelectTile), "PlaceMode")
            << transition("PlaceMode", GetEventName(EventType::ExitMode), "BuildMode")
            << transition("PlaceMode", GetEventName(EventType::EnterDeleteMode), "DeleteMode")
            << transition("DeleteMode", GetEventName(EventType::SelectTile), "PlaceMode"); 

        fsm.logger = [this](const FSM&, const std::string& from_state, const Event& event, const std::string& to_state) {
            logger.info("Event {} sent from state {} to {}", event.nameAsString(), from_state, to_state);
        };

        fsm.start().setState("Idle");
    }

    State StateIdle(FSM& fsm) {
        auto event = co_await fsm.getEvent();

        while (true) {
            logger.info("In Idle!");

            if (EventMatches(event, EventType::ExitMode)) {
                EmitStateCommand<commands::StateExitMode>();
            }

            event.destroy();
            event = co_await fsm.emitAndReceive(&event);
        }
    }

    State StateBuildMode(FSM& fsm) {
        auto event = co_await fsm.getEvent();

        while (true) {
            logger.info("In BuildMode!");
            
            if (event == GetEventName(EventType::KeyPress)) {
                const KeyPress* press{};
                event >> press;

                logger.info(
                    "Key {} pressed in place mode",
                    press->key
                );

                if (press->key == GLFW_KEY_ESCAPE) {
                    ReplaceEvent(event, EventType::ExitMode);
                    event = co_await fsm.emitAndReceive(&event);

                    continue;
                }
            }

            event.destroy();
            event = co_await fsm.emitAndReceive(&event);
        }
    }

    State StatePlaceMode(FSM& fsm) {
        auto event = co_await fsm.getEvent();
        logger.info("Top: {}", (void*) &event);

        TileType selected_tile{};

        while (true) {
            logger.info("In PlaceMode!");

            logger.info("Loop: {}", (void*) &event);
            if (event == GetEventName(EventType::SelectTile)) {
                TileType* selected_tile_ptr{};
                event >> selected_tile_ptr;

                selected_tile = *selected_tile_ptr;
                logger.info("Selected tile - {}: {}", (void*) &selected_tile, static_cast<int>(selected_tile));
            } else if (event == GetEventName(EventType::Click)) {
                MouseClick* click{};
                event >> click;

                logger.info("Selected tile - {}: {}", (void*) &selected_tile, static_cast<int>(selected_tile));
                logger.info(
                    "Place tile {} ({}: {}) at {}, {}",
                    magic_enum::enum_name(selected_tile),
                    (void*) &selected_tile,
                    static_cast<int>(selected_tile),
                    click->pos.x, 
                    click->pos.y
                );
            } else if (event == GetEventName(EventType::KeyPress)) {
                KeyPress* press{};
                event >> press;

                logger.info(
                    "Key {} pressed in place mode",
                    press->key
                );

                if (press->key == GLFW_KEY_ESCAPE) {
                    ReplaceEvent(event, EventType::ExitMode);
                    event = co_await fsm.emitAndReceive(&event);
                    continue;
                }
            }

            event.destroy();
            event = co_await fsm.emitAndReceive(&event);
        }
    }

    State StateDeleteMode(FSM& fsm) {
        auto event = co_await fsm.getEvent();

        while (true) {
            logger.info("In DeleteMode!");

            if (event == GetEventName(EventType::Click)) {
                MouseClick* click{};
                event >> click;

                logger.info("Delete tile at {}, {}", click->pos.x, click->pos.y);
            } else if (event == GetEventName(EventType::KeyPress)) {
                KeyPress* press{};
                event >> press;

                logger.info(
                    "Key {} pressed in place mode",
                    press->key
                );

                if (press->key == GLFW_KEY_ESCAPE) {
                    ReplaceEvent(event, EventType::ExitMode);
                    event = co_await fsm.emitAndReceive(&event);
                    continue;
                }
            }

            event.destroy();
            event = co_await fsm.emitAndReceive(&event);
        }
    }

    template<typename CommandType, typename... Args>
    void EmitStateCommand(Args&&... args) {
        // state_cmd_rec->PushCommand(
        //     commands::MakeCommandPtr<CommandType>(std::forward<Args>(args)...)
        // );
        state_cmd_prod->Send(MakeCommandPtr<CommandType>(std::forward<Args>(args)...));
    }
};

DEFINE_PIMPL_DELETER(BuildMode);

BuildMode::BuildMode()
    : m_pimpl(new Pimpl(state_commands_out))
{ }

DEFINE_PIMPL_SMFS(BuildMode);


void BuildMode::Consume(Action&& action) {
    std::visit([this]<typename T>(const T& a) -> void {
        if constexpr (std::is_same_v<T, MouseClick>) {
            Event event{};
            ReplaceEvent(event, EventType::Click, a);
            m_pimpl->fsm.sendEvent(&event);
        } else if constexpr (std::is_same_v<T, KeyPress>) {
            Event event{};
            ReplaceEvent(event, EventType::KeyPress, a);
            m_pimpl->fsm.sendEvent(&event);
        }
    }, action.data);
}

void BuildMode::Consume(commands::BuildModeCommandPtr&& command) {
    command->Execute(*this);
}

void BuildMode::EnterBuildMode() {
    Event e;
    ReplaceEvent(e, EventType::EnterBuildMode);
    m_pimpl->fsm.sendEvent(&e);
}

void BuildMode::ExitMode() {
    Event e;
    ReplaceEvent(e, EventType::ExitMode);
    m_pimpl->fsm.sendEvent(&e);
}

void BuildMode::SelectTile(TileType tile) {
    Event e;
    ReplaceEvent(e, EventType::SelectTile, tile);
    m_pimpl->fsm.sendEvent(&e);
}

void BuildMode::BeginDeleting() {
    Event e;
    ReplaceEvent(e, EventType::EnterDeleteMode);
    m_pimpl->fsm.sendEvent(&e);
}

void BuildMode::EndDeleting() {
    Event e;
    ReplaceEvent(e, EventType::ExitMode);
    m_pimpl->fsm.sendEvent(&e);
}

auto BuildMode::GetState() const -> StateType {
    auto cur_state = m_pimpl->fsm.currentState();

    if (cur_state == "Idle") {
        return StateType::Idle;
    } else if (cur_state == "BuildMode") {
        return StateType::BuildMode;
    } else if (cur_state == "PlaceMode") {
        return StateType::PlaceMode;
    } else if (cur_state == "DeleteMode") {
        return StateType::DeleteMode;
    } else {
        throw std::runtime_error(std::format("Unknown state in BuildMode: {}", cur_state));
    }
}