#pragma once

#include <common/ui/UiEvents.hpp>
#include <modules/ui/Panel.hpp>
#include <modules/input/InputState.hpp>
#include <modules/input/RoomInput.hpp>

#include <magic_enum/magic_enum.hpp>

namespace ui {

class RoomPanel : public Panel {
    application::EventManager* m_event_manager{};

public:
    RoomPanel(application::EventManager& em)
        : Panel("room-panel"), m_event_manager(&em) {

        em.GetOrRegisterSignal<ui_events::ElementEvent>()
            .signal.connect<&RoomPanel::ProcessElementEvent>(this);
    }

    void ProcessElementEvent(const ui_events::ElementEvent& event) {
        const auto& args = event.args;
        
        if (args.size() == 0)
            return;

        if (args[0] == "SelectRoomClear") {
            m_logger.info("Sending SelectRoomClear");

            m_event_manager->GetSignal<input::RoomInputCommand>()
                .signal.fire([](input::RoomInput& ri) {
                        ri.SelectRoomClear();
                    });
        } else if (args[0] == "SelectRoomType") {
            if (args.size() != 2)
                throw std::runtime_error("SelectRoomType requires 1 argument");

            m_logger.info("Sending SelectRoomType: {}", args[1]);
            auto room_type = magic_enum::enum_cast<core::RoomType>(std::stoi(args[1]));

            if (!room_type.has_value()) {
                throw std::runtime_error(std::format("Invalid argument for SelectRoomType: {}", args[1]));
            }

            m_event_manager->GetSignal<input::RoomInputCommand>()
                .signal.fire([room_type = room_type.value()](input::RoomInput& ri) {
                        ri.SelectRoomType(room_type);
                    });
        } else if (args[0] == "EnterRoomMode") {
            m_logger.info("Sending EnterRoomMode");

            m_event_manager->GetSignal<input::InputStateCommand>()
                .signal.fire([](input::InputState& is) {
                        is.EnterRoomMode();
                    });
        } else if (args[0] == "ExitRoomMode") {
            m_logger.info("Sending ExitRoomMode");

            m_event_manager->GetSignal<input::InputStateCommand>()
                .signal.fire([](input::InputState& is) {
                        is.ExitMode();
                    });
        }
    }
};

}