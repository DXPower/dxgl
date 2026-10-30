#include <modules/input/BuildInput.hpp>
#include <modules/core/BuildManager.hpp>
#include <common/Logging.hpp>

#include <magic_enum/magic_enum.hpp>
#include <GLFW/glfw3.h>

#include <glm/gtx/string_cast.hpp>

#include <dxfsm/dxfsm.hpp>
#include <glm/matrix.hpp>

using namespace input;
using namespace core;

BuildInput::BuildInput(application::EventManager& em, IdRegistry& registry)
    : m_em(&em), m_editing_tool(em, registry) {

    On(m_editing_tool.m_action_select_type, [this](InputTool&, const ToolEvent& ev, ToolEventControl&) {
        EnterChild(m_editing_tool);
        m_editing_tool.Dispatch(ev);
    });
    
    On(m_editing_tool.m_action_clear, [this](InputTool&, const ToolEvent& ev, ToolEventControl&) {
        EnterChild(m_editing_tool);
        m_editing_tool.Dispatch(ev);
    });
}

void BuildInput::OnEnter() {
    m_logger.info("Entered build input tool");
    m_em->FireSignal(BuildInputStateChanged{
        .to = BuildInputStates::IdleMode
    });
}

void BuildInput::OnExit() {
    m_logger.info("Exiting build input tool");
    m_em->FireSignal(BuildInputStateChanged{
        .to = BuildInputStates::InactiveMode
    });
}

BuildInput::EditingTool::EditingTool(application::EventManager& em, IdRegistry& registry) 
    : m_em(&em) {
    m_action_clear = registry.AddId("BuildInput.SelectTileClear");
    m_action_select_type = registry.AddId("BuildInput.SelectTileType");
    Hotkeys().BindKey(GLFW_KEY_R, m_action_clear);
    
    On(m_action_clear, [this](InputTool&, const ToolEvent&, ToolEventControl&) {
        m_selected_type.reset();
    });

    On(m_action_select_type, [this](InputTool&, const ToolEvent& event, ToolEventControl&) {
        const auto& select = GetPayloadAs<SelectTileType>(event.payload);
        m_selected_type = select.type;
    });

    On(ToolEvent::Builtin::Cancel, [this](InputTool&, const ToolEvent&, ToolEventControl& control) {
        if (m_selected_type.has_value()) {
            m_selected_type.reset();
        } else {
            control.PropagateUp();
        }
    });

    On(ToolEvent::Builtin::DragComplete, [this](InputTool&, const ToolEvent& ev, ToolEventControl&) {
        const auto& drag = GetPayloadAs<DragPayload>(ev.payload);
        
        if (!drag.begin.tile_coord.has_value() || !drag.end.tile_coord.has_value()) {
            return;
        }

        EditTiles({.start = *drag.begin.tile_coord, .end = *drag.end.tile_coord});
    });

    On(ToolEvent::Builtin::Tap, [this](InputTool&, const ToolEvent& ev, ToolEventControl&) {
        const auto& click = GetPayloadAs<MousePayload>(ev.payload);

        if (!click.tile_coord.has_value()) {
            return;
        }

        EditTiles({.start = *click.tile_coord, .end = *click.tile_coord});
    });
}

void BuildInput::EditingTool::OnEnter() {
    m_logger.info("Entered editing tool");
    m_selected_type.reset();

    m_em->FireSignal(BuildInputStateChanged{
        .to = BuildInputStates::PlaceTileMode
    });
}

void BuildInput::EditingTool::OnExit() {
    m_logger.info("Exiting editing tool");
    m_em->FireSignal(BuildInputStateChanged{
        .to = BuildInputStates::IdleMode
    });
}

void BuildInput::EditingTool::EditTiles(const core::TileSelection& selection) {
    if (m_selected_type.has_value()) {
        m_logger.info("Editing tiles from {} to {} with type {}", glm::to_string(selection.start), glm::to_string(selection.end), magic_enum::enum_name(*m_selected_type));

        m_em->FireSignal(BuildCommand{[selection, type = *m_selected_type](BuildManager& bm) {
            for (auto coord : selection.Iterate()) {
                bm.PlaceTile(coord, type);
            }
        }});
    } else {
        m_logger.info("Clearing tiles from {} to {}", glm::to_string(selection.start), glm::to_string(selection.end));

        m_em->FireSignal(BuildCommand{[selection](BuildManager& bm) {
            for (auto coord : selection.Iterate()) {
                bm.DeleteTopmostTile(coord, TileLayer::Walls);
            }
        }});
    }
}
