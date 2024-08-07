#include <services/BuildInput.hpp>
#include <services/Logging.hpp>

#include <magic_enum.hpp>
#include <glfw/glfw3.h>

#include <dxfsm/dxfsm.hpp>

using namespace services;
using namespace commands;

namespace {
    using StateId = BuildInput::StateId;

    enum class EventId {
        ExitMode,
        EnterBuildMode,
        EnterDeleteMode,
        SelectTileToPlace,
        SelectWorldTile,
        Click,
        KeyPress
    };

    struct SelectWorldTile {
        TileCoord coords{};
    };
    
    using FSM_t = dxfsm::FSM<StateId, EventId>;
    using State_t = FSM_t::State_t;
    using Event_t = FSM_t::Event_t;
}

class BuildInput::Pimpl {
public:
    FSM_t fsm{};
    logging::Logger logger{logging::CreateLogger("BuildInput")};
    CommandProducer<StateCommand>* state_cmd_prod{};

    Pimpl(CommandProducer<StateCommand>& state_cmd_prod)
        : state_cmd_prod(&state_cmd_prod)
    {
        logger.set_level(spdlog::level::debug);
        
        using enum StateId;
        using enum EventId;

        StateIdle(fsm, IdleMode);
        StateBuild(fsm, BuildMode);
        StatePlaceTile(fsm, PlaceTileMode);
        StateWorldTileSelected(fsm, WorldTileSelectedMode);
        StateDelete(fsm, DeleteMode);
        
        fsm.SetCurrentState(IdleMode);

        // Exit transitions
        // WorldTileSelected -> Build -> Idle -> (State exit command)
        // PlaceTile --------^
        // Delete ----------^

        fsm.AddTransition(BuildMode, ExitMode, IdleMode);
        fsm.AddTransition(WorldTileSelectedMode, ExitMode, BuildMode);
        fsm.AddTransition(PlaceTileMode, ExitMode, BuildMode);
        fsm.AddTransition(DeleteMode, ExitMode, BuildMode);

        // EnterBuildMode - this can be generated by keypresses so we need
        // the event wired up
        fsm.AddTransition(IdleMode, EnterBuildMode, BuildMode);
        fsm.AddTransition(WorldTileSelectedMode, EnterBuildMode, BuildMode);
        fsm.AddTransition(DeleteMode, EnterBuildMode, BuildMode);

        // EnterDeleteMode - this can be generated by keypresses so we need
        // the event wired up
        fsm.AddTransition(BuildMode, EnterDeleteMode, DeleteMode);
        fsm.AddTransition(PlaceTileMode, EnterDeleteMode, DeleteMode);

        fsm.AddTransition(BuildMode, SelectTileToPlace, PlaceTileMode);
        fsm.AddTransition(WorldTileSelectedMode, SelectTileToPlace, PlaceTileMode);
        fsm.AddTransition(DeleteMode, SelectTileToPlace, PlaceTileMode);
        
        fsm.AddTransition(BuildMode, SelectWorldTile, WorldTileSelectedMode);
    }

    State_t StateIdle(FSM_t& fsm, StateId) {
        Event_t event = co_await fsm.ReceiveInitialEvent();

        while (true) {
            logger.debug("In StateIdle");
            
            if (event == EventId::ExitMode) {
                EmitStateCommand<commands::StateExitMode>();
                logger.info("Send ExitBuildMode state command!");
            }

            co_await fsm.ReceiveEvent(event);
        }
    }

    State_t StateBuild(FSM_t& fsm, StateId) {
        Event_t event{};

        while (true) {
            co_await fsm.EmitAndReceive(event);
            logger.debug("In StateBuild");

            if (event == EventId::KeyPress) {
                const auto& press = event.Get<KeyPress>();

                if (press.dir == ButtonDir::Down && press.key == GLFW_KEY_R) {
                    logger.info("Keyboard shortcut to go to delete mode");
                    event = EventId::EnterDeleteMode;
                    continue;
                }
            }

            event.Clear();
        }
    }

    State_t StatePlaceTile(FSM_t& fsm, StateId) {
        Event_t event{};
        TileType selected_tile{};

        while (true) {
            co_await fsm.EmitAndReceive(event);
            logger.debug("In StatePlaceTile");

            if (event == EventId::SelectTileToPlace) {
                selected_tile = event.Get<TileType>();
                logger.info("Selected tile: {}", static_cast<int>(selected_tile));
            } else if (event == EventId::Click) {
                const auto& click = event.Get<MouseClick>();

                if (click.button == 0 && click.dir == ButtonDir::Down) {
                    logger.info(
                        "Place tile {} ({}) at {}, {}",
                        magic_enum::enum_name(selected_tile),
                        static_cast<int>(selected_tile),
                        click.pos.x, 
                        click.pos.y
                    );
                }
            } else if (event == EventId::KeyPress) {
                const auto& press = event.Get<KeyPress>();

                if (press.key == GLFW_KEY_R) {
                    event = EventId::EnterDeleteMode;
                    continue;
                }
                // rotate?
            }

            event.Clear();
        }
    }

    State_t StateWorldTileSelected(FSM_t& fsm, StateId) {
        Event_t event{};
        TileCoord selected_tile{};

        while (true) {
            co_await fsm.ReceiveEvent(event);
            logger.debug("In StateWorldTileSelected");

            if (event == EventId::SelectWorldTile) {
                const auto& tile = event.Get<SelectWorldTile>();
                selected_tile = tile.coords;

                logger.info(
                    "Selecting world tile at coords {}, {}",
                    tile.coords.x, tile.coords.y
                );
            } else if (event == EventId::Click) {
                const auto& click = event.Get<MouseClick>();

                if (click.button == 0 && click.dir == ButtonDir::Down) {
                    logger.info(
                        "Selecting world tile at world pos {}, {}",
                        click.pos.x, click.pos.y
                    );
                }
            } else if (event == EventId::KeyPress) {
                const auto& press = event.Get<KeyPress>();

                if (press.key == GLFW_KEY_D && press.dir == ButtonDir::Down) {
                    logger.info(
                        "Deleting tile at coords {}, {}",
                        selected_tile.x, selected_tile.y
                    );

                    // Go back to build mode after deleting the tile
                    event = EventId::EnterBuildMode;
                    continue;
                }
                // rotate?
            }

            event.Clear();
        }
    }

    State_t StateDelete(FSM_t& fsm, StateId) {
        Event_t event{};

        while (true) {
            co_await fsm.ReceiveEvent(event);
            logger.debug("In StateDelete"); 

            if (event == EventId::Click) {
                const auto& click = event.Get<MouseClick>();
                logger.info("Delete tile at {}, {}", click.pos.x, click.pos.y);
            } else if (event == EventId::KeyPress) {
                const auto& press = event.Get<KeyPress>();

                logger.info(
                    "Key {} pressed in delete mode",
                    press.key
                );

                if (press.key == GLFW_KEY_B) {
                    logger.info("Keyboard shortcut to go to build mode");
                    event = EventId::EnterBuildMode;
                    continue;
                }
            }

            event.Clear();
        }
    }

    template<typename CommandType, typename... Args>
    void EmitStateCommand(Args&&... args) {
        state_cmd_prod->Send(MakeCommandPtr<CommandType>(std::forward<Args>(args)...));
    }
};

DEFINE_PIMPL_DELETER(BuildInput);

BuildInput::BuildInput()
    : m_pimpl(new Pimpl(state_commands_out))
{ }

DEFINE_PIMPL_SMFS(BuildInput);


void BuildInput::Consume(Action&& action) {
    std::visit([this]<typename T>(const T& a) -> void {
        if constexpr (std::is_same_v<T, MouseClick>) {
            m_pimpl->fsm.InsertEvent(EventId::Click, a);
        } else if constexpr (std::is_same_v<T, KeyPress>) {
            const KeyPress& press = a;

            if (press.key == GLFW_KEY_ESCAPE && press.dir == ButtonDir::Down) {
                ExitMode();
            } else {
                m_pimpl->fsm.InsertEvent(EventId::KeyPress, a);
            }
        }
    }, action.data);
}

void BuildInput::Consume(commands::BuildInputCommandPtr&& command) {
    command->Execute(*this);
}

void BuildInput::EnterBuildMode() {
    m_pimpl->fsm.InsertEvent(EventId::EnterBuildMode);
}

void BuildInput::EnterDeleteMode() {
    m_pimpl->fsm.InsertEvent(EventId::EnterDeleteMode);
}

void BuildInput::SelectTileToPlace(TileType tile) {
    m_pimpl->fsm.InsertEvent(EventId::SelectTileToPlace, tile);
}

void BuildInput::ExitMode() {
    m_pimpl->logger.info("Sending exit mode...");
    m_pimpl->fsm.InsertEvent(EventId::ExitMode);
}

auto BuildInput::GetState() const -> StateId {
    return m_pimpl->fsm.GetCurrentState()->Id();
}