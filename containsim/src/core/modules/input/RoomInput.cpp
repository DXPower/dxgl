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

    On(ToolEvent::Builtin::DragComplete, [this](InputTool&, const ToolEvent& ev, ToolEventControl&) {
        const auto& drag = GetPayloadAs<DragPayload>(ev.payload);

        if (!drag.begin.tile_coord.has_value() || !drag.end.tile_coord.has_value()) {
            return;
        }

        MarkTiles({.start = *drag.begin.tile_coord, .end = *drag.end.tile_coord});
    });

    On(ToolEvent::Builtin::Tap, [this](InputTool&, const ToolEvent& ev, ToolEventControl&) {
        const auto& click = GetPayloadAs<MousePayload>(ev.payload);

        if (!click.tile_coord.has_value()) {
            return;
        }

        MarkTiles({.start = *click.tile_coord, .end = *click.tile_coord});
    });
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

void RoomInput::DemarcationTool::MarkTiles(const core::TileSelection& selection) {
    if (!m_selected_type.has_value()) {
        m_logger.info("Sending clear command for selection {} to {}", glm::to_string(selection.start), glm::to_string(selection.end));
        
        m_em->FireSignal<RoomCommand>([selection](RoomManager& rm) {
                rm.UnmarkTiles(selection);
            });
    } else {
        m_logger.info("Sending mark command from {} to {} as room type {}", glm::to_string(selection.start), glm::to_string(selection.end), magic_enum::enum_name(*m_selected_type));

        m_em->FireSignal<RoomCommand>([selection, type = *m_selected_type](RoomManager& rm) {
                rm.MarkTilesAsRoom(selection, type);
            });
    }
}
