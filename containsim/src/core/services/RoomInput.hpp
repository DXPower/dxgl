#pragma once

#include <common/Room.hpp>
#include <services/EventManager.hpp>

#include <common/ActionChain.hpp>
#include <services/RoomInputEvents.hpp>
#include <services/commands/CommandChains.hpp>
#include <services/commands/RoomInputCommands.hpp>
#include <services/commands/RoomCommands.hpp>

#include <dxfsm/dxfsm.hpp>
#include <services/Logging.hpp>

#include <modules/rendering/Camera.hpp>
#include <services/TileGrid.hpp>

namespace services {
    class RoomInput 
        : public ActionConsumer
    {
    public:
        using StateId = RoomInputStates;
        
        enum class EventId {
            Entry,
            ExitMode,
            SelectRoomType,
            SelectRoomClear,
            Click,
            KeyPress
        };
        
        ActionProducer uncaptured_actions{};
        commands::CommandProducer<commands::RoomCommand> room_commands{};

    private:
        using State_t = dxfsm::State<StateId>;
        using Event_t = dxfsm::Event<EventId>;
        using FSM_t = dxfsm::FSM<StateId, EventId>;

        FSM_t m_fsm{};
        EventManager* m_event_manager{};
        const rendering::Camera* m_camera{};
        const TileGrid* m_tiles{};
        logging::Logger m_logger = logging::CreateLogger("RoomInput");

    public:
        RoomInput(EventManager& em, const rendering::Camera& cam, const TileGrid& tiles);

        void Consume(Action&& action) override;

        void SelectRoomType(RoomType room);
        void SelectRoomClear();
        void ExitMode();

        auto& GetFsm(this auto&& self) { return self.m_fsm;}

        StateId GetState() const { return m_fsm.GetCurrentState()->Id(); };

    private:
        State_t StateIdle(FSM_t& fsm, StateId);
        State_t StateDemarcation(FSM_t& fsm, StateId);

        void ProcessRoomInputCommand(const commands::RoomInputCommand& cmd);

        std::optional<TileCoord> ScreenToTilePos(glm::vec2 screen_pos) const;
    };
}