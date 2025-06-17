#include <services/RoomInput.hpp>
#include <services/Logging.hpp>

#include <magic_enum/magic_enum.hpp>
#include <GLFW/glfw3.h>

#include <dxfsm/dxfsm.hpp>
#include <glm/matrix.hpp>

using namespace services;
using namespace commands;

RoomInput::RoomInput(EventManager& em, const rendering::Camera& cam, const TileGrid& tiles)
    : m_event_manager(&em), m_camera(&cam), m_tiles(&tiles) {
    m_logger.set_level(spdlog::level::debug);
    
    using enum StateId;
    using enum EventId;

    StateIdle(m_fsm, IdleMode);
    StateDemarcation(m_fsm, DemarcationMode);
    
    m_fsm.SetCurrentState(IdleMode);

    m_fsm.AddTransition(DemarcationMode, ExitMode, IdleMode);
    m_fsm.AddTransition(IdleMode, SelectRoomType, DemarcationMode);
    m_fsm.AddTransition(IdleMode, SelectRoomClear, DemarcationMode);

    em.GetOrRegisterSignal<commands::RoomInputCommand>()
        .signal.connect<&RoomInput::ProcessRoomInputCommand>(this);

    m_fsm.SetTransitionObserver([this](const FSM_t&, std::optional<State_t> from, State_t to, const Event_t& ev) {
        std::string ev_str = !ev.Empty() ? std::format(" Event: {}", magic_enum::enum_name(ev.GetId())) : "";
        
        if (!from.has_value()) {
            m_logger.info("Transitioning to {}.{}", magic_enum::enum_name(to.Id()), ev_str);
        } else {
            m_logger.info(
                "Transitioning from {} to {}.{}",
                magic_enum::enum_name(from->Id()),
                magic_enum::enum_name(to.Id()),
                ev_str
            );
        }

        m_event_manager->GetSignal<RoomInputStateChanged>()
            .signal.fire(RoomInputStateChanged{to.Id()});
    });
}

auto RoomInput::StateIdle(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};

    while (true) {
        co_await fsm.EmitAndReceive(event);

        if (event == EventId::KeyPress) {
            const auto& press = event.Get<KeyPress>();
            uncaptured_actions.Send(Action{press});
        } else if (event == EventId::Click) {
            const auto& click = event.Get<MouseClick>();
            uncaptured_actions.Send(Action{click});
        }

        event.Clear();
    }
}

auto RoomInput::StateDemarcation(FSM_t& fsm, StateId) -> State_t {
    Event_t event{};
    std::optional<RoomType> selected_room{};

    TileCoord drag_start{};
    bool drag_valid = false;

    while (true) {
        co_await fsm.EmitAndReceive(event);

        if (event == EventId::SelectRoomType) {
            selected_room = event.Get<RoomType>();
            m_logger.info("Selected room: {}", static_cast<int>(*selected_room));
        } else if (event == EventId::SelectRoomClear) {
            selected_room.reset();
            m_logger.info("Cleared room selection");
        } else if (event == EventId::Click) {
            const auto& click = event.Get<MouseClick>();

            if (click.button == 0 && click.dir == ButtonDir::Down) {
                const auto tile_pos = ScreenToTilePos(click.pos);

                if (tile_pos.has_value()) {
                    drag_start = *tile_pos;
                    drag_valid = true;
                } else {
                    drag_valid = false;
                }
            } else if (click.button == 0 && click.dir == ButtonDir::Up && drag_valid) {
                const auto tile_pos = ScreenToTilePos(click.pos);

                if (tile_pos.has_value()) {
                    if (selected_room.has_value()) {
                        auto cmd = commands::MakeCommandPtr<commands::MarkRoom>();
                        cmd->tiles.start = drag_start;
                        cmd->tiles.end = *tile_pos;
                        cmd->type = *selected_room;
                        
                        room_commands.Send(std::move(cmd));
                    } else {
                        auto cmd = commands::MakeCommandPtr<commands::UnmarkRoom>();
                        cmd->tiles.start = drag_start;
                        cmd->tiles.end = *tile_pos;
                        
                        room_commands.Send(std::move(cmd));
                    }
                }

                drag_valid = false;
            } else {
                uncaptured_actions.Send(Action{click});
            }
        } else if (event == EventId::KeyPress) {
            const auto& press = event.Get<KeyPress>();

            if (press.key == GLFW_KEY_R) {
                event = EventId::SelectRoomClear;
                continue;
            } else {
                uncaptured_actions.Send(Action{press});
            }
        }

        event.Clear();
    }
}

void RoomInput::Consume(Action&& action) {
    std::visit([&action, this]<typename T>(const T& a) -> void {
        if constexpr (std::is_same_v<T, MouseClick>) {
            m_fsm.InsertEvent(EventId::Click, a);
        } else if constexpr (std::is_same_v<T, KeyPress>) {
            const KeyPress& press = a;

            if (press.key == GLFW_KEY_ESCAPE && press.dir == ButtonDir::Down) {
                ExitMode();
            } else {
                m_fsm.InsertEvent(EventId::KeyPress, a);
            }
        } else {
            uncaptured_actions.Send(std::move(action));
        }
    }, action.data);
}

void RoomInput::SelectRoomType(RoomType room) {
    m_fsm.InsertEvent(EventId::SelectRoomType, room);
}

void RoomInput::SelectRoomClear() {
    m_fsm.InsertEvent(EventId::SelectRoomClear);
}

void RoomInput::ExitMode() {
    m_fsm.InsertEvent(EventId::ExitMode);
}

void RoomInput::ProcessRoomInputCommand(const commands::RoomInputCommand& cmd) {
    cmd.Execute(*this);
}

std::optional<TileCoord> RoomInput::ScreenToTilePos(glm::vec2 screen_pos) const {
    const auto cam_view = m_camera->GetViewMatrix();
    const auto world_pos = glm::inverse(cam_view) * glm::vec4(screen_pos, 1, 1);
    return m_tiles->WorldPosToTileCoord(world_pos);
}
