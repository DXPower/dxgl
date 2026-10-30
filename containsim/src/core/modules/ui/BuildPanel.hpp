#pragma once

#include <modules/input/BuildInput.hpp>
#include <modules/input/RootTool.hpp>

#include <modules/ui/UiEvents.hpp>
#include <modules/ui/Panel.hpp>

#include <magic_enum/magic_enum.hpp>

namespace ui {

class BuildPanel : public Panel {
    application::EventManager* m_event_manager{};
    input::RootTool* m_root_tool{};
public:
    BuildPanel(application::EventManager& em, input::RootTool& root_tool)
        : Panel("build-panel"), m_event_manager(&em), m_root_tool(&root_tool) {

        em.GetOrRegisterSignal<ui_events::ElementEvent>()
            .signal.connect<&BuildPanel::ProcessElementEvent>(this);
    }

    void ProcessElementEvent(const ui_events::ElementEvent& event) {
        const auto& args = event.args;
        
        if (args.size() == 0)
            return;

        if (args[0] == "SelectTileToPlace") {
            if (args.size() != 2)
                throw std::runtime_error("SelectTileToPlace requires 1 argument");

            m_logger.info("SelectTileToPlace: {}", args[1]);
            auto tile_type = static_cast<core::TileType>(std::stoi(args[1]));

            input::ToolEvent tool_ev{};
            tool_ev.id = m_root_tool->Registry().GetOrAddId("BuildInput.SelectTileType");
            tool_ev.payload = input::AnyPayload{
                .data = input::BuildInput::SelectTileType{
                    .type = tile_type
                }
            };
            m_root_tool->Dispatch(tool_ev);
        } else if (args[0] == "EnterBuildMode") {
            m_logger.info("Making BuildInput active...");

            input::ToolEvent tool_ev{};
            tool_ev.id = m_root_tool->Registry().GetOrAddId("RootTool.EnterBuildTool");
            m_root_tool->Dispatch(tool_ev);
        } else if (args[0] == "ExitBuildMode") {
            m_logger.info("Making BuildInput inactive...");

            m_root_tool->ReturnToSelf();
        }
    }
};

}