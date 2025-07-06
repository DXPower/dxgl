#pragma once

#include <common/Room.hpp>
#include <modules/application/EventManager.hpp>

#include <common/ActionChain.hpp>
#include <common/RoomInputEvents.hpp>
#include <services/commands/CommandChains.hpp>
#include <services/commands/RoomInputCommands.hpp>
#include <services/commands/RoomCommands.hpp>

#include <dxfsm/dxfsm.hpp>
#include <services/Logging.hpp>

#include <modules/rendering/Camera.hpp>
#include <modules/core/TileGrid.hpp>

namespace input {
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
        services::commands::CommandProducer<services::commands::RoomCommand> room_commands{};

    private:
        using State_t = dxfsm::State<StateId>;
        using Event_t = dxfsm::Event<EventId>;
        using FSM_t = dxfsm::FSM<StateId, EventId>;

        FSM_t m_fsm{};
        application::EventManager* m_event_manager{};
        const rendering::Camera* m_camera{};
        const core::TileGrid* m_tiles{};
        services::logging::Logger m_logger = services::logging::CreateLogger("RoomInput");

    public:
        RoomInput(application::EventManager& em, const rendering::Camera& cam, const core::TileGrid& tiles);

        void Consume(Action&& action) override;

        void SelectRoomType(RoomType room);
        void SelectRoomClear();
        void ExitMode();

        auto& GetFsm(this auto&& self) { return self.m_fsm;}

        StateId GetState() const { return m_fsm.GetCurrentState()->Id(); };

    private:
        State_t StateIdle(FSM_t& fsm, StateId);
        State_t StateDemarcation(FSM_t& fsm, StateId);

        void ProcessRoomInputCommand(const services::commands::RoomInputCommand& cmd);

        std::optional<TileCoord> ScreenToTilePos(glm::vec2 screen_pos) const;
    };
}