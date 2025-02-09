#pragma once

// #include <services/commands/BuildInputCommands.hpp>
#include <common/Tile.hpp>
#include <services/EventManager.hpp>

#include <common/ActionChain.hpp>
#include <services/commands/CommandChains.hpp>
#include <services/commands/BuildInputCommands.hpp>

#include <dxfsm/dxfsm.hpp>
#include <services/Logging.hpp>

namespace services {
    class BuildInput 
        : public ActionConsumer
        //   public commands::CommandConsumer<commands::BuildInputCommand>
    {
    public:
        enum class StateId {
            IdleMode,
            PlaceTileMode,
            WorldTileSelectedMode,
            DeleteMode
        };
        
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

    private:
        using State_t = dxfsm::State<StateId>;
        using Event_t = dxfsm::Event<EventId>;
        using FSM_t = dxfsm::FSM<StateId, EventId>;

        FSM_t m_fsm{};
        logging::Logger m_logger = logging::CreateLogger("BuildInput");

    public:
        BuildInput(EventManager& em);

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
    };
}