#pragma once

#include <modules/core/Tile.hpp>
#include <modules/application/EventManager.hpp>
#include <modules/rendering/Camera.hpp>
#include <modules/core/TileGrid.hpp>

#include <common/ActionChain.hpp>
#include <common/BuildInputEvents.hpp>
#include <common/EventCommandable.hpp>
#include <common/Logging.hpp>

#include <dxfsm/dxfsm.hpp>

namespace input {
    class BuildInput 
        : public ActionConsumer
        , public EventCommandable<BuildInput>
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

    private:
        using State_t = dxfsm::State<StateId>;
        using Event_t = dxfsm::Event<EventId>;
        using FSM_t = dxfsm::FSM<StateId, EventId>;

        FSM_t m_fsm{};
        application::EventManager* m_event_manager{};
        const rendering::Camera* m_camera{};
        const core::TileGrid* m_tiles{};
        logging::Logger m_logger = logging::CreateLogger("BuildInput");

    public:
        BuildInput(application::EventManager& em, const rendering::Camera& cam, const core::TileGrid& tiles);

        void Consume(Action&& action) override;

        void EnterDeleteMode();
        void SelectTileToPlace(core::TileType tile);
        void ExitMode();

        auto GetFsm() -> FSM_t& { return m_fsm; }
        auto GetFsm() const -> const FSM_t& { return m_fsm; }

        StateId GetState() const { return m_fsm.GetCurrentState()->Id(); };

    private:
        State_t StateIdle(FSM_t& fsm, StateId);
        State_t StatePlaceTile(FSM_t& fsm, StateId);
        State_t StateWorldTileSelected(FSM_t& fsm, StateId);
        State_t StateDelete(FSM_t& fsm, StateId);

        std::optional<core::TileCoord> ScreenToTilePos(glm::vec2 screen_pos) const;
    };

    using BuildInputCommand = Command<BuildInput>;
}