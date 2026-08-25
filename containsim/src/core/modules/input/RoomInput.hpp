#pragma once

// #include <modules/input/DragHelper.hpp>
// #include <modules/input/HotkeyMap.hpp>
#include <modules/core/Room.hpp>
#include <modules/application/EventManager.hpp>

// #include <common/ActionChain.hpp>
#include <common/RoomInputEvents.hpp>

// #include <dxfsm/dxfsm.hpp>
#include <common/Logging.hpp>
// #include <common/EventCommandable.hpp>

// #include <modules/rendering/Camera.hpp>
// #include <modules/core/TileGrid.hpp>
// #include <common/MeceFsm.hpp>

#include <modules/input/InputTool.hpp>

namespace input {

class RoomInput final : public InputTool {
    logging::Logger m_logger{logging::CreateLogger("RoomInput")};
    application::EventManager* m_em{};

    class DemarcationTool final : public InputTool {
        logging::Logger m_logger{logging::CreateLogger("RoomInput.DemarcationTool")};
        application::EventManager* m_em{};

        std::optional<core::RoomType> m_selected_type{};
    public:
        int m_action_select_type{};
        int m_action_clear{};

        DemarcationTool(application::EventManager& em, IdRegistry& registry);

    protected:
        void OnEnter() override;
        void OnExit() override;

        void OnClick(InputTool&, const ToolEvent& event, ToolEventControl&);
    } m_demarcation_tool;

public:
    RoomInput(application::EventManager& em, IdRegistry& registry);

    void OnEnter() override;
    void OnExit() override;

    struct SelectRoomType {
        core::RoomType type{};
    };
};

}

// namespace input {
//     class RoomInput final 
//         : public EventCommandable<RoomInput>
//         , public MeceSubFsm
//     {
//     public:
//         struct StateId {
//             inline static int Demarcation{};
//         };

//         struct EventId {
//             inline static int ExitMode{};
//             inline static int SelectRoomType{};
//             inline static int SelectRoomClear{};
//             inline static int Action{};
//         };
        
//         ActionProducer uncaptured_actions{};

//     private:
//         application::EventManager* m_event_manager{};
//         const rendering::Camera* m_camera{};
//         const core::TileGrid* m_tiles{};
//         std::unique_ptr<DragHelper> m_drag_helper{};

//         HotkeyMap m_hotkeys{};
//         int m_action_exit{};
//         int m_action_clear{};
//         int m_action_click{};
//     public:
//         RoomInput(application::EventManager& em, const rendering::Camera& cam, const core::TileGrid& tiles);

//         void SelectRoomType(core::RoomType room);
//         void SelectRoomClear();
//         void ExitMode();

//     private:
//         State StateIdle(FSM& fsm, int) override;
//         State StateDemarcation(FSM& fsm, int);

//         std::optional<core::TileCoord> ScreenToTilePos(glm::vec2 screen_pos) const;
        
//         void OnStateChanged(const FSM&, std::optional<State>, State to, const Event&) override;
//     };

//     using RoomInputCommand = Command<RoomInput>;
// }