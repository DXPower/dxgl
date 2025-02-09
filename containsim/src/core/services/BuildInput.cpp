#include <services/BuildInput.hpp>
#include <services/Logging.hpp>

#include <magic_enum/magic_enum.hpp>
#include <GLFW/glfw3.h>

#include <dxfsm/dxfsm.hpp>

using namespace services;
using namespace commands;

namespace {
    struct SelectWorldTile {
        TileCoord coords{};
    };
}


BuildInput::BuildInput(EventManager& em) {
    m_logger.set_level(spdlog::level::debug);
    
    using enum StateId;
    using enum EventId;

    StateIdle(m_fsm, IdleMode);
    StatePlaceTile(m_fsm, PlaceTileMode);
    StateWorldTileSelected(m_fsm, WorldTileSelectedMode);
    StateDelete(m_fsm, DeleteMode);
    
    m_fsm.SetCurrentState(IdleMode);

    // Exit transitions
    // WorldTileSelected -> Idle
    // PlaceTile --------^
    // Delete ----------^

    m_fsm.AddTransition(WorldTileSelectedMode, ExitMode, IdleMode);
    m_fsm.AddTransition(PlaceTileMode, ExitMode, IdleMode);
    m_fsm.AddTransition(DeleteMode, ExitMode, IdleMode);

    // // EnterBuildMode - this can be generated by keypresses so we need
    // // the event wired up
    // m_fsm.AddTransition(WorldTileSelectedMode, EnterBuildMode, EntryMode);
    // m_fsm.AddTransition(DeleteMode, EnterBuildMode, EntryMode);

    // EnterDeleteMode - this can be generated by keypresses so we need
    // the event wired up
    m_fsm.AddTransition(IdleMode, BeginDeleting, DeleteMode);
    m_fsm.AddTransition(PlaceTileMode, BeginDeleting, DeleteMode);

    // SelectTileToPlace
    m_fsm.AddTransition(IdleMode, SelectTileToPlace, PlaceTileMode);
    m_fsm.AddTransition(WorldTileSelectedMode, SelectTileToPlace, PlaceTileMode);
    m_fsm.AddTransition(DeleteMode, SelectTileToPlace, PlaceTileMode);
    
    // SelectWorldTile
    m_fsm.AddTransition(IdleMode, SelectWorldTile, WorldTileSelectedMode);

    em.GetOrRegisterSignal<commands::BuildInputCommand>()
        .signal.connect<&BuildInput::ProcessBuiltInputCommand>(this);
}

// auto BuildInput::StateIdle(FSM_t& fsm, StateId) -> State_t {
//         Event_t event = co_await fsm.ReceiveInitialEvent();

//         while (true) {
//             m_logger.debug("In StateIdle");
            
//             if (event == EventId::ExitMode) {
//                 EmitStateCommand<commands::StateExitMode>();
//                 m_logger.info("Send ExitBuildMode state command!");
//             }

//             co_await fsm.ReceiveEvent(event);
//         }
//     }

auto BuildInput::StateIdle(FSM_t& fsm, StateId) -> State_t {
    Event_t event = co_await fsm.ReceiveInitialEvent();

    while (true) {
        m_logger.debug("In StateBuild");

        if (event == EventId::KeyPress) {
            const auto& press = event.Get<KeyPress>();

            if (press.dir == ButtonDir::Down && press.key == GLFW_KEY_R) {
                m_logger.info("Keyboard shortcut to go to delete mode");
                event = EventId::BeginDeleting;
                co_await fsm.EmitAndReceive(event);
                continue;
            } else {
                uncaptured_actions.Send(Action{press});
            }
        }

        co_await fsm.ReceiveEvent(event);
    }
}

auto BuildInput::StatePlaceTile(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};
    TileType selected_tile{};

    while (true) {
        co_await fsm.EmitAndReceive(event);
        m_logger.debug("In StatePlaceTile");

        if (event == EventId::SelectTileToPlace) {
            selected_tile = event.Get<TileType>();
            m_logger.info("Selected tile: {}", static_cast<int>(selected_tile));
        } else if (event == EventId::Click) {
            const auto& click = event.Get<MouseClick>();

            if (click.button == 0 && click.dir == ButtonDir::Down) {
                m_logger.info(
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
                event = EventId::BeginDeleting;
                continue;
            } else {
                uncaptured_actions.Send(Action{press});
            }
            // rotate?
        }

        event.Clear();
    }
}

auto BuildInput::StateWorldTileSelected(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};
    TileCoord selected_tile{};

    while (true) {
        co_await fsm.ReceiveEvent(event);
        m_logger.debug("In StateWorldTileSelected");

        if (event == EventId::SelectWorldTile) {
            const auto& tile = event.Get<SelectWorldTile>();
            selected_tile = tile.coords;

            m_logger.info(
                "Selecting world tile at coords {}, {}",
                tile.coords.x, tile.coords.y
            );
        } else if (event == EventId::Click) {
            const auto& click = event.Get<MouseClick>();

            if (click.button == 0 && click.dir == ButtonDir::Down) {
                m_logger.info(
                    "Selecting world tile at world pos {}, {}",
                    click.pos.x, click.pos.y
                );
            }
        } else if (event == EventId::KeyPress) {
            const auto& press = event.Get<KeyPress>();

            if (press.key == GLFW_KEY_D && press.dir == ButtonDir::Down) {
                m_logger.info(
                    "Deleting tile at coords {}, {}",
                    selected_tile.x, selected_tile.y
                );

                // Go back to build mode after deleting the tile
                event = EventId::ExitMode;
                continue;
            } else {
                uncaptured_actions.Send(Action{press});
            }
            // rotate?
        }

        event.Clear();
    }
}

auto BuildInput::StateDelete(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};

    while (true) {
        co_await fsm.ReceiveEvent(event);
        m_logger.debug("In StateDelete"); 

        if (event == EventId::Click) {
            const auto& click = event.Get<MouseClick>();
            m_logger.info("Delete tile at {}, {}", click.pos.x, click.pos.y);
        } else if (event == EventId::KeyPress) {
            const auto& press = event.Get<KeyPress>();

            m_logger.info(
                "Key {} pressed in delete mode",
                press.key
            );

            if (press.key == GLFW_KEY_B) {
                m_logger.info("Keyboard shortcut to go to build mode");
                event = EventId::ExitMode;
                continue;
            } else {
                uncaptured_actions.Send(Action{press});
            }
        }

        event.Clear();
    }
}

void BuildInput::Consume(Action&& action) {
    std::visit([&action, this]<typename T>(const T& a) -> void {
        if constexpr (std::is_same_v<T, MouseClick>) {
            m_fsm.InsertEvent(EventId::Click, a);
        } else if constexpr (std::is_same_v<T, KeyPress>) {
            const KeyPress& press = a;

            if (press.key == GLFW_KEY_ESCAPE && press.dir == ButtonDir::Down) {
                ExitMode();
            } else {
                m_fsm.InsertEvent(EventId::KeyPress, a);
            }
        } else if constexpr (std::is_same_v<T, MouseMove>) {
            uncaptured_actions.Send(std::move(action));
        }
    }, action.data);
}

void BuildInput::EnterDeleteMode() {
    m_fsm.InsertEvent(EventId::BeginDeleting);
}

void BuildInput::SelectTileToPlace(TileType tile) {
    m_fsm.InsertEvent(EventId::SelectTileToPlace, tile);
}

void BuildInput::ExitMode() {
    m_logger.info("Sending exit mode...");
    m_fsm.InsertEvent(EventId::ExitMode);
}

void BuildInput::ProcessBuiltInputCommand(const commands::BuildInputCommand& cmd) {
    cmd.Execute(*this);
}
