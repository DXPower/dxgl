#pragma once

#include <modules/input/BuildInput.hpp>
#include <modules/input/InputState.hpp>

#include <modules/ui/UiEvents.hpp>
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
            int tile_type = std::stoi(args[1]);

            m_event_manager->GetSignal<input::BuildInputCommand>()
                .signal.fire([tile_type](input::BuildInput& fsm) {
                        fsm.SelectTileToPlace(tile_type);
                    });
        } else if (args[0] == "EnterBuildMode") {
            m_logger.info("Sending EnterBuildMode");

            m_event_manager->FireSignal<input::BuildInputCommand>([](input::BuildInput& bi) {
                bi.MakeActive();
            });
        } else if (args[0] == "ExitBuildMode") {
            m_logger.info("Sending ExitBuildMode");

            m_event_manager->FireSignal<input::BuildInputCommand>([](input::BuildInput& bi) {
                bi.MakeInactive();
            });
        }
    }
};

}