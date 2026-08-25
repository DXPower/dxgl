#include <modules/input/RoomInput.hpp>
#include <modules/core/RoomManager.hpp>
#include <common/Logging.hpp>

#include <magic_enum/magic_enum.hpp>
#include <GLFW/glfw3.h>

#include <glm/gtx/string_cast.hpp>

#include <dxfsm/dxfsm.hpp>
#include <glm/matrix.hpp>

using namespace input;
using namespace core;

RoomInput::RoomInput(application::EventManager& em, IdRegistry& registry)
    : m_em(&em), m_demarcation_tool(em, registry) {

    On(m_demarcation_tool.m_action_select_type, [this](InputTool&, const ToolEvent& ev, ToolEventControl&) {
        EnterChild(m_demarcation_tool);
        m_demarcation_tool.Dispatch(ev);
    });
    
    On(m_demarcation_tool.m_action_clear, [this](InputTool&, const ToolEvent& ev, ToolEventControl&) {
        EnterChild(m_demarcation_tool);
        m_demarcation_tool.Dispatch(ev);
    });
}

void RoomInput::OnEnter() {
    m_logger.info("Entered room input tool");
    m_em->FireSignal(RoomInputStateChanged{
        .to = RoomInputStates::IdleMode
    });
}

void RoomInput::OnExit() {
    m_logger.info("Exiting room input tool");
    m_em->FireSignal(RoomInputStateChanged{
        .to = RoomInputStates::InactiveMode
    });
}

RoomInput::DemarcationTool::DemarcationTool(application::EventManager& em, IdRegistry& registry) 
    : m_em(&em) {
    m_action_clear = registry.AddId("RoomInput.SelectRoomClear");
    m_action_select_type = registry.AddId("RoomInput.SelectRoomType");
    Hotkeys().BindKey(GLFW_KEY_R, m_action_clear);
    
    On(m_action_clear, [this](InputTool&, const ToolEvent&, ToolEventControl&) {
        m_selected_type.reset();
    });

    On(m_action_select_type, [this](InputTool&, const ToolEvent& event, ToolEventControl&) {
        const auto& select = GetPayloadAs<SelectRoomType>(event.payload);
        m_selected_type = select.type;
    });

    On(ToolEvent::Builtin::Click, std::bind_front(&DemarcationTool::OnClick, this));
}

void RoomInput::DemarcationTool::OnEnter() {
    m_logger.info("Entered demarcation tool");
    m_selected_type.reset();

    m_em->FireSignal(RoomInputStateChanged{
        .to = RoomInputStates::DemarcationMode
    });
}

void RoomInput::DemarcationTool::OnExit() {
    m_logger.info("Exited demarcation tool");
    m_em->FireSignal(RoomInputStateChanged{
        .to = RoomInputStates::IdleMode
    });
}

void RoomInput::DemarcationTool::OnClick(InputTool&, const ToolEvent& event, ToolEventControl&) {
    const auto& click = GetPayloadAs<ClickPayload>(event.payload);

    if (!click.tile_coord.has_value()) {
        m_logger.info("Click at {} is not on a tile", glm::to_string(click.screen_pos));
        return;
    }

    if (!m_selected_type.has_value()) {
        m_logger.info("Attempt to clear room at {}", glm::to_string(*click.tile_coord));
    } else {
        m_logger.info("Attempt to mark tile at {} as room type {}", glm::to_string(*click.tile_coord), magic_enum::enum_name(*m_selected_type));
    }

    // m_logger.info("Marking tile at {} as room type {}", glm::to_string(click->tile_coord), magic_enum::enum_name(*m_selected_type));
}

// RoomInput::RoomInput(application::EventManager& em, const rendering::Camera& cam, const core::TileGrid& tiles)
//     : EventCommandable<RoomInput>(em)
//     , MeceSubFsm("RoomInput")
//     , m_event_manager(&em), m_camera(&cam), m_tiles(&tiles) {
//     m_logger.set_level(spdlog::level::debug);

//     InitializeDefaultStates();

//     auto& fsm = GetFsm();

//     auto demarcation = AddState("StateDemarcation", std::bind_front(&RoomInput::StateDemarcation, this));
//     StateId::Demarcation = demarcation.Id();

//     auto& events = GetEventInfo();
//     EventId::ExitMode = events.AddId("ExitMode");
//     EventId::SelectRoomType = events.AddId("SelectRoomType");
//     EventId::SelectRoomClear = events.AddId("SelectRoomClear");
//     EventId::Action = events.AddId("Action");

//     fsm.AddTransition(StateId::Demarcation, EventId::ExitMode, MeceSubStates::Idle);
//     fsm.AddTransition(MeceSubStates::Idle, EventId::ExitMode, MeceSubStates::Inactive);
//     fsm.AddTransition(MeceSubStates::Idle, EventId::SelectRoomType, StateId::Demarcation);
//     fsm.AddTransition(MeceSubStates::Idle, EventId::SelectRoomClear, StateId::Demarcation);

//     m_drag_helper = std::make_unique<DragHelper>(fsm, GetStateInfo(), GetEventInfo(), StateId::Demarcation, "DemarcationDrag");

//     AddExitTransitionsToAllStates();

//     m_action_exit = events.AddId("ActionExit");
//     m_action_clear = events.AddId("ActionClear");
//     m_action_click = events.AddId("ActionClick");
//     m_hotkeys.BindKey(GLFW_KEY_ESCAPE, m_action_exit);
//     m_hotkeys.BindKey(GLFW_KEY_R, m_action_clear);
//     m_hotkeys.BindMouse(GLFW_MOUSE_BUTTON_LEFT, m_action_click);
// }

// auto RoomInput::StateIdle(FSM& fsm, int self) -> State {
//     Event event{};

//     while (true) {
//         co_await fsm.EmitAndReceive(event);

//         if (event == EventId::Action) {
//             // TODO: Redo how uncaptured inputs are handled
//             // uncaptured_actions.Send(Action{press});
//             const auto& action = event.Get<Action>();

//             if (auto action_id = m_hotkeys.TryTranslate(action); action_id == m_action_exit) {
//                 event = MeceSubEvents::ExitingSubFsm;
//                 continue;
//             }
//         } else if (auto prev = fsm.GetPreviousState(); event == EventId::ExitMode && prev.has_value() && prev->Id() == self) {
//             event = MeceSubEvents::ExitingSubFsm;
//             continue;
//         }

//         event.Clear();
//     }
// }

// auto RoomInput::StateDemarcation(FSM& fsm, int) -> State {
//     Event event{};
//     std::optional<RoomType> selected_room{};

//     while (true) {
//         co_await fsm.EmitAndReceive(event);

//         if (event == EventId::SelectRoomType) {
//             selected_room = event.Get<RoomType>();
//             m_logger.info("Selected room: {}", static_cast<int>(*selected_room));
//         } else if (event == EventId::SelectRoomClear) {
//             selected_room.reset();
//             m_logger.info("Cleared room selection");
//         } else if (event == m_drag_helper->GetDragCompletedId()) {
//             const auto& drag_data = event.Get<DragData>();
//             const auto start_tile = std::any_cast<TileCoord>(drag_data.user_data);
//             const auto end_tile = ScreenToTilePos(drag_data.end.pos);

//             if (end_tile.has_value()) {
//                 TileSelection sel{start_tile, *end_tile};

//                 if (selected_room.has_value()) {
//                     m_event_manager->FireSignal<RoomCommand>([sel, type = *selected_room](RoomManager& rm) {
//                             rm.MarkTilesAsRoom(sel, type);
//                         });
//                 } else {
//                     m_event_manager->FireSignal<RoomCommand>([sel](RoomManager& rm) {
//                             rm.UnmarkTiles(sel);
//                         });
//                 }
//             }
//         } else if (event == EventId::Action) {
//             const auto& action = event.Get<Action>();
//             const auto action_id = m_hotkeys.TryTranslate(action);

//             if (action_id == m_action_click) {
//                 const auto& click = std::get<MouseClick>(action.data);
//                 auto clicked_tile_coord = ScreenToTilePos(click.pos);

//                 if (!clicked_tile_coord.has_value()) {
//                     event.Clear();
//                     continue;
//                 }

//                 DragStartedData drag_data{};
//                 drag_data.begin = click;
//                 drag_data.user_data = *clicked_tile_coord;
//                 event.Store(m_drag_helper->GetDragStartedId(), std::move(drag_data));
//                 continue;
//             } else if (action_id == m_action_exit) {
//                     event = EventId::ExitMode;
//                     continue;
//             } else if (action_id == m_action_clear) {
//                 event = EventId::SelectRoomClear;
//                 continue;
//             }
//         } 

//         event.Clear();
//     }
// }

// void RoomInput::SelectRoomType(RoomType room) {
//     GetFsm().InsertEvent(EventId::SelectRoomType, room);
// }

// void RoomInput::SelectRoomClear() {
//     GetFsm().InsertEvent(EventId::SelectRoomClear);
// }

// void RoomInput::ExitMode() {
//     GetFsm().InsertEvent(EventId::ExitMode);
// }

// std::optional<TileCoord> RoomInput::ScreenToTilePos(glm::vec2 screen_pos) const {
//     const auto cam_view = m_camera->GetViewMatrix();
//     const auto world_pos = glm::inverse(cam_view) * glm::vec4(screen_pos, 1, 1);
//     return m_tiles->WorldPosToTileCoord(world_pos);
// }

// void RoomInput::OnStateChanged(const FSM&, std::optional<State>, State to, const Event&) {
//     const auto check = to.Id();
//     RoomInputStates res{};
    
//     if (check == MeceSubStates::Inactive) {
//         res =  RoomInputStates::InactiveMode;
//     } else if (check == MeceSubStates::Idle) {
//         res =  RoomInputStates::IdleMode;
//     } else if (check == StateId::Demarcation) {
//         res =  RoomInputStates::DemarcationMode;
//     } else if (check == m_drag_helper->GetDragStateId()) {
//         res = RoomInputStates::DemarcationDragMode;
//     }

//     m_event_manager->FireSignal(RoomInputStateChanged{res});
// }
