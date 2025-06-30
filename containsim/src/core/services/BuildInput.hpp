#pragma once

// #include <services/commands/BuildInputCommands.hpp>
#include <common/Tile.hpp>
#include <services/EventManager.hpp>

#include <common/ActionChain.hpp>
#include <services/BuildInputEvents.hpp>
#include <services/commands/CommandChains.hpp>
#include <services/commands/BuildInputCommands.hpp>
#include <services/commands/BuildCommands.hpp>

#include <dxfsm/dxfsm.hpp>
#include <services/Logging.hpp>

#include <modules/rendering/Camera.hpp>
#include <modules/core/TileGrid.hpp>

namespace services {
    class BuildInput 
        : public ActionConsumer
    {
    public:
        using StateId = BuildInputStates;
        
        enum class EventId {
            Entry,
            ExitMode,
            BeginDeleting,
            SelectTileToPlace,
            SelectWorldTile,
            Click,
            KeyPress
        };
        
        ActionProducer uncaptured_actions{};
        commands::CommandProducer<commands::BuildCommand> build_commands{};

    private:
        using State_t = dxfsm::State<StateId>;
        using Event_t = dxfsm::Event<EventId>;
        using FSM_t = dxfsm::FSM<StateId, EventId>;

        FSM_t m_fsm{};
        EventManager* m_event_manager{};
        const rendering::Camera* m_camera{};
        const core::TileGrid* m_tiles{};
        logging::Logger m_logger = logging::CreateLogger("BuildInput");

    public:
        BuildInput(EventManager& em, const rendering::Camera& cam, const core::TileGrid& tiles);

        void Consume(Action&& action) override;
        // void Consume(commands::BuildInputCommandPtr&& command) override;

        void EnterDeleteMode();
        void SelectTileToPlace(TileType tile);
        void ExitMode();

        auto GetFsm() -> FSM_t& { return m_fsm; }
        auto GetFsm() const -> const FSM_t& { return m_fsm; }

        StateId GetState() const { return m_fsm.GetCurrentState()->Id(); };

    private:
        State_t StateIdle(FSM_t& fsm, StateId);
        State_t StatePlaceTile(FSM_t& fsm, StateId);
        State_t StateWorldTileSelected(FSM_t& fsm, StateId);
        State_t StateDelete(FSM_t& fsm, StateId);

        void ProcessBuiltInputCommand(const commands::BuildInputCommand& cmd);

        std::optional<TileCoord> ScreenToTilePos(glm::vec2 screen_pos) const;
    };
}