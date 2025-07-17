#pragma once

#include <modules/input/DragHelper.hpp>
#include <modules/core/Room.hpp>
#include <modules/application/EventManager.hpp>

#include <common/ActionChain.hpp>
#include <common/RoomInputEvents.hpp>

#include <dxfsm/dxfsm.hpp>
#include <common/Logging.hpp>
#include <common/EventCommandable.hpp>

#include <modules/rendering/Camera.hpp>
#include <modules/core/TileGrid.hpp>
#include <common/MeceFsm.hpp>

namespace input {
    class RoomInput final 
        : public EventCommandable<RoomInput>
        , public MeceSubFsm
    {
    public:
        struct StateId {
            inline static int Demarcation{};
        };

        struct EventId {
            inline static int ExitMode{};
            inline static int SelectRoomType{};
            inline static int SelectRoomClear{};
            inline static int Action{};
        };
        
        ActionProducer uncaptured_actions{};

    private:
        application::EventManager* m_event_manager{};
        const rendering::Camera* m_camera{};
        const core::TileGrid* m_tiles{};
        std::unique_ptr<DragHelper> m_drag_helper{};

    public:
        RoomInput(application::EventManager& em, const rendering::Camera& cam, const core::TileGrid& tiles);

        void SelectRoomType(core::RoomType room);
        void SelectRoomClear();
        void ExitMode();

    private:
        State StateIdle(FSM& fsm, int) override;
        State StateDemarcation(FSM& fsm, int);

        std::optional<core::TileCoord> ScreenToTilePos(glm::vec2 screen_pos) const;
        
        void OnStateChanged(const FSM&, std::optional<State>, State to, const Event&) override;
    };

    using RoomInputCommand = Command<RoomInput>;
}