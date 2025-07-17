#include <modules/input/RoomInput.hpp>
#include <modules/core/RoomManager.hpp>
#include <common/Logging.hpp>

#include <magic_enum/magic_enum.hpp>
#include <GLFW/glfw3.h>

#include <dxfsm/dxfsm.hpp>
#include <glm/matrix.hpp>

using namespace input;
using namespace core;

RoomInput::RoomInput(application::EventManager& em, const rendering::Camera& cam, const core::TileGrid& tiles)
    : EventCommandable<RoomInput>(em)
    , MeceSubFsm("RoomInput")
    , m_event_manager(&em), m_camera(&cam), m_tiles(&tiles) {
    m_logger.set_level(spdlog::level::debug);

    InitializeDefaultStates();

    auto& fsm = GetFsm();

    auto demarcation = AddState("StateDemarcation", std::bind_front(&RoomInput::StateDemarcation, this));
    StateId::Demarcation = demarcation.Id();

    auto& events = GetEventInfo();
    EventId::ExitMode = events.AddId("ExitMode");
    EventId::SelectRoomType = events.AddId("SelectRoomType");
    EventId::SelectRoomClear = events.AddId("SelectRoomClear");
    EventId::Action = events.AddId("Action");

    fsm.AddTransition(StateId::Demarcation, EventId::ExitMode, MeceSubStates::Idle);
    fsm.AddTransition(MeceSubStates::Idle, EventId::ExitMode, MeceSubStates::Inactive);
    fsm.AddTransition(MeceSubStates::Idle, EventId::SelectRoomType, StateId::Demarcation);
    fsm.AddTransition(MeceSubStates::Idle, EventId::SelectRoomClear, StateId::Demarcation);

    m_drag_helper = std::make_unique<DragHelper>(fsm, GetStateInfo(), GetEventInfo(), StateId::Demarcation, "DemarcationDrag");

    AddExitTransitionsToAllStates();
}

auto RoomInput::StateIdle(FSM& fsm, int self) -> State {
    Event event{};

    while (true) {
        co_await fsm.EmitAndReceive(event);

        if (event == EventId::Action) {
            // TODO: Redo how uncaptured inputs are handled
            // uncaptured_actions.Send(Action{press});
            const auto& action = event.Get<Action>();
            const auto* press = std::get_if<KeyPress>(&action.data);

            if (press != nullptr) {
                if (press->IsDownKey(GLFW_KEY_ESCAPE)) {
                    event = MeceSubEvents::ExitingSubFsm;
                    continue;
                }
            }
        } else if (auto prev = fsm.GetPreviousState(); event == EventId::ExitMode && prev.has_value() && prev->Id() == self) {
            event = MeceSubEvents::ExitingSubFsm;
            continue;
        }

        event.Clear();
    }
}

auto RoomInput::StateDemarcation(FSM& fsm, int) -> State {
    Event event{};
    std::optional<RoomType> selected_room{};

    while (true) {
        co_await fsm.EmitAndReceive(event);

        if (event == EventId::SelectRoomType) {
            selected_room = event.Get<RoomType>();
            m_logger.info("Selected room: {}", static_cast<int>(*selected_room));
        } else if (event == EventId::SelectRoomClear) {
            selected_room.reset();
            m_logger.info("Cleared room selection");
        } else if (event == m_drag_helper->GetDragCompletedId()) {
            const auto& drag_data = event.Get<DragData>();
            const auto start_tile = ScreenToTilePos(drag_data.begin.pos);
            const auto end_tile = ScreenToTilePos(drag_data.end.pos);

            if (start_tile.has_value() && end_tile.has_value()) {
                TileSelection sel{*start_tile, *end_tile};

                if (selected_room.has_value()) {
                    m_event_manager->FireSignal<RoomCommand>([sel, type = *selected_room](RoomManager& rm) {
                            rm.MarkTilesAsRoom(sel, type);
                        });
                } else {
                    m_event_manager->FireSignal<RoomCommand>([sel](RoomManager& rm) {
                            rm.UnmarkTiles(sel);
                        });
                }
            }
        } else if (event == EventId::Action) {
            const auto& action = event.Get<Action>();
            const MouseClick* click = std::get_if<MouseClick>(&action.data);
            const KeyPress* press = std::get_if<KeyPress>(&action.data);

            if (click != nullptr) {
                if (click->button == 0 && click->dir == ButtonDir::Down) {
                    event.Store(m_drag_helper->GetDragStartedId(), DragStartedData{*click});
                    continue;
                }
            } else if (press != nullptr) {
                if (press->IsDownKey(GLFW_KEY_ESCAPE)) {
                    event = EventId::ExitMode;
                    continue;
                } else if (press->IsDownKey(GLFW_KEY_R)) {
                    event = EventId::SelectRoomClear;
                    continue;
                }
            }
        } 

        event.Clear();
    }
}

void RoomInput::SelectRoomType(RoomType room) {
    GetFsm().InsertEvent(EventId::SelectRoomType, room);
}

void RoomInput::SelectRoomClear() {
    GetFsm().InsertEvent(EventId::SelectRoomClear);
}

void RoomInput::ExitMode() {
    GetFsm().InsertEvent(EventId::ExitMode);
}

std::optional<TileCoord> RoomInput::ScreenToTilePos(glm::vec2 screen_pos) const {
    const auto cam_view = m_camera->GetViewMatrix();
    const auto world_pos = glm::inverse(cam_view) * glm::vec4(screen_pos, 1, 1);
    return m_tiles->WorldPosToTileCoord(world_pos);
}

void RoomInput::OnStateChanged(const FSM&, std::optional<State>, State to, const Event&) {
    const auto check = to.Id();
    RoomInputStates res{};
    
    if (check == MeceSubStates::Inactive) {
        res =  RoomInputStates::InactiveMode;
    } else if (check == MeceSubStates::Idle) {
        res =  RoomInputStates::IdleMode;
    } else if (check == StateId::Demarcation) {
        res =  RoomInputStates::DemarcationMode;
    } else if (check == m_drag_helper->GetDragStateId()) {
        res = RoomInputStates::DemarcationDragMode;
    }

    m_event_manager->FireSignal(RoomInputStateChanged{res});
}
