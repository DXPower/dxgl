#pragma once

#include <modules/input/DragHelper.hpp>
#include <modules/core/Tile.hpp>
#include <modules/application/EventManager.hpp>
#include <modules/rendering/Camera.hpp>
#include <modules/core/TileGrid.hpp>

#include <common/ActionChain.hpp>
#include <common/BuildInputEvents.hpp>
#include <common/EventCommandable.hpp>
#include <common/Logging.hpp>
#include <common/MeceFsm.hpp>

#include <dxfsm/dxfsm.hpp>

namespace input {
    class BuildInput final
        : public EventCommandable<BuildInput>
        , public MeceSubFsm
    {
    public:
        struct StateId {
            inline static int PlaceTileMode{};
            inline static int DeleteMode{};

            StateId() = delete;
        };

        struct EventId {
            inline static int ExitMode{};
            inline static int BeginDeleting{};
            inline static int SelectTileToPlace{};
            inline static int SelectWorldTile{};
            inline static int Action{};

            EventId() = delete;
        };
        
        ActionProducer uncaptured_actions{};
        application::EventManager* m_event_manager{};
        const rendering::Camera* m_camera{};
        const core::TileGrid* m_tiles{};
        std::unique_ptr<DragHelper> m_drag_helper{};
        
        BuildInput(application::EventManager& em, const rendering::Camera& cam, const core::TileGrid& tiles);

        void EnterDeleteMode();
        void SelectTileToPlace(core::TileType tile);
        void ExitMode();

    private:
        State StateIdle(FSM& fsm, int) override;
        State StatePlaceTile(FSM& fsm, int);
        State StateDelete(FSM& fsm, int);

        std::optional<core::TileCoord> ScreenToTilePos(glm::vec2 screen_pos) const;

        void OnStateChanged(const FSM&, std::optional<State>, State to, const Event&) override;
    };

    using BuildInputCommand = Command<BuildInput>;
}