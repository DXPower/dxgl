#pragma once

#include <common/ui/UiEvents.hpp>
#include <services/ui/Panel.hpp>
#include <services/commands/RoomInputCommands.hpp>
#include <services/commands/InputStateCommands.hpp>

#include <magic_enum/magic_enum.hpp>

namespace services {
namespace ui {

class RoomPanel : public Panel {
    EventManager* m_event_manager{};

public:
    RoomPanel(EventManager& em, Rml::ElementDocument& document)
        : Panel("room-panel", em, document), m_event_manager(&em) {

        em.GetOrRegisterSignal<ui_events::ElementEvent>()
            .signal.connect<&RoomPanel::ProcessElementEvent>(this);
    }

    void ProcessElementEvent(const ui_events::ElementEvent& event) {
        const auto& args = event.args;
        
        if (args.size() == 0)
            return;

        if (args[0] == "SelectRoomClear") {
            m_logger.info("Sending SelectRoomClear");

            commands::SelectRoomClear cmd{};
            m_event_manager->GetSignal<commands::RoomInputCommand>()
                .signal.fire(cmd);
        } else if (args[0] == "SelectRoomType") {
            if (args.size() != 2)
                throw std::runtime_error("SelectRoomType requires 1 argument");

            m_logger.info("Sending SelectRoomType: {}", args[1]);
            auto room_type = magic_enum::enum_cast<RoomType>(std::stoi(args[1]));

            if (!room_type.has_value()) {
                throw std::runtime_error(std::format("Invalid argument for SelectRoomType: {}", args[1]));
            }

            commands::SelectRoomType cmd{};
            cmd.type = *room_type;
            m_event_manager->GetSignal<commands::RoomInputCommand>()
                .signal.fire(cmd);
        } else if (args[0] == "EnterRoomMode") {
            m_logger.info("Sending EnterRoomMode");

            m_event_manager->GetSignal<commands::InputStateCommand>()
                .signal.fire(commands::InputStateEnterRoomMode{});
        } else if (args[0] == "ExitRoomMode") {
            m_logger.info("Sending ExitRoomMode");

            m_event_manager->GetSignal<commands::InputStateCommand>()
                .signal.fire(commands::InputStateExitMode{});
        }
    }
};

}
}