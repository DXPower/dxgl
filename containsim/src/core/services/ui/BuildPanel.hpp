#pragma once

#include <common/ui/UiEvents.hpp>
#include <services/ui/Panel.hpp>
#include <services/commands/BuildInputCommands.hpp>
#include <services/commands/InputStateCommands.hpp>

namespace services {
namespace ui {

class BuildPanel : public Panel {
    core::EventManager* m_event_manager{};
    // commands::CommandProducer<commands::BuildInputCommand> build_input_cmds{};

public:
    BuildPanel(core::EventManager& em, Rml::ElementDocument& document)
        : Panel("build-panel", em, document), m_event_manager(&em) {

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
            auto tile_type = magic_enum::enum_cast<TileType>(args[1]);

            if (!tile_type.has_value()) {
                throw std::runtime_error(std::format("Invalid argument for SelectTileToPlace: {}", args[1]));
            }

            commands::SelectTile cmd{};
            cmd.type = *tile_type;
            m_event_manager->GetSignal<commands::BuildInputCommand>()
                .signal.fire(cmd);
        } else if (args[0] == "EnterBuildMode") {
            // build_input_cmds.Send(std::make_unique<commands::EnterBuildMode>());
            m_logger.info("Sending EnterBuildMode");

            m_event_manager->GetSignal<commands::InputStateCommand>()
                .signal.fire(commands::InputStateEnterBuildMode{});
        } else if (args[0] == "ExitBuildMode") {
            // build_input_cmds.Send(std::make_unique<commands::ExitBuildMode>());
            // m_event_manager->GetSignal<commands::InputStateCommand>()
            //     .signal.fire(commands::InputStateExitMode{});
            m_logger.info("Sending ExitBuildMode");

            m_event_manager->GetSignal<commands::InputStateCommand>()
                .signal.fire(commands::InputStateExitMode{});
        }
        // if (args[0] == "SelectTileToPlace") {
        //     if (args.size() != 2)
        //         throw std::runtime_error("SelectTileToPlace requires 1 argument");

        //     auto cmd = std::make_unique<commands::SelectTile>();
        //     cmd->type = (TileType)std::stoi(args[1]);
        //     build_input_cmds.Send(std::move(cmd));
        //     m_logger.info("SelectTileToPlace: {}", args[1]);

        //     return EventProc::Consumed;
        // } else if (args[0] == "EnterBuildMode") {
        //     // build_input_cmds.Send(std::make_unique<commands::EnterBuildMode>());
        //     m_logger.info("UI sending EnterBuildMode");
        //     m_event_manager->GetSignal<commands::InputStateCommand>()
        //         .signal.fire(commands::InputStateEnterBuildMode{});

        //     return EventProc::Consumed;
        // } else if (args[0] == "ExitBuildMode") {
        //     // build_input_cmds.Send(std::make_unique<commands::ExitBuildMode>());
        //     // m_event_manager->GetSignal<commands::InputStateCommand>()
        //     //     .signal.fire(commands::InputStateExitMode{});
        //     m_logger.info("ExitBuildMode");

        //     return EventProc::Consumed;
        // }
    }
};

}
}