#pragma once

#include <modules/input/BuildInput.hpp>
#include <modules/input/InputState.hpp>

#include <common/ui/UiEvents.hpp>
#include <modules/ui/Panel.hpp>

#include <magic_enum/magic_enum.hpp>

namespace ui {

class BuildPanel : public Panel {
    application::EventManager* m_event_manager{};

public:
    BuildPanel(application::EventManager& em)
        : Panel("build-panel"), m_event_manager(&em) {

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
            auto tile_type = magic_enum::enum_cast<core::TileType>(args[1]);

            if (!tile_type.has_value()) {
                throw std::runtime_error(std::format("Invalid argument for SelectTileToPlace: {}", args[1]));
            }

            m_event_manager->GetSignal<input::BuildInputCommand>()
                .signal.fire([tile_type = tile_type.value()](input::BuildInput& fsm) {
                        fsm.SelectTileToPlace(tile_type);
                    });
        } else if (args[0] == "EnterBuildMode") {
            m_logger.info("Sending EnterBuildMode");

            m_event_manager->GetSignal<input::InputStateCommand>()
                .signal.fire([](input::InputState& is) {
                        is.EnterBuildMode();
                    });
        } else if (args[0] == "ExitBuildMode") {
            m_logger.info("Sending ExitBuildMode");

            m_event_manager->GetSignal<input::InputStateCommand>()
                .signal.fire([](input::InputState& is) {
                        is.ExitMode();
                    });
        }
    }
};

}