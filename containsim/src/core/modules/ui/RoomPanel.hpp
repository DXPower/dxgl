#pragma once

#include <modules/ui/UiEvents.hpp>
#include <modules/ui/Panel.hpp>
#include <modules/input/InputState.hpp>
#include <modules/input/RootTool.hpp>
#include <modules/input/RoomInput.hpp>
#include <modules/core/RoomTypeMeta.hpp>
#include <common/ResourceRegistry.hpp>

#include <magic_enum/magic_enum.hpp>

namespace ui {

class RoomPanel : public Panel {
    [[maybe_unused]] application::EventManager* m_event_manager{};
    const ResourceRegistry* m_resources{};
    input::RootTool* m_root_tool{};
public:
    RoomPanel(application::EventManager& em, const ResourceRegistry& resources, input::RootTool& root_tool)
        : Panel("room-panel"), m_event_manager(&em), m_resources(&resources), m_root_tool(&root_tool) {

        em.GetOrRegisterSignal<ui_events::ElementEvent>()
            .signal.connect<&RoomPanel::ProcessElementEvent>(this);
    }

    void ProcessElementEvent(const ui_events::ElementEvent& event) {
        using namespace input;

        const auto& args = event.args;
        
        if (args.size() == 0)
            return;

        if (args[0] == "SelectRoomClear") {
            m_logger.info("Sending SelectRoomClear");

            // m_event_manager->FireSignal<RoomInputCommand>([](RoomInput& ri) {
            //     ri.SelectRoomClear();
            // });
            input::ToolEvent tool_ev{};
            tool_ev.id = m_root_tool->Registry().GetOrAddId("RoomInput.SelectRoomClear");
            m_root_tool->Dispatch(tool_ev);
        } else if (args[0] == "SelectRoomType") {
            if (args.size() != 2)
                throw std::runtime_error("SelectRoomType requires 1 argument");

            m_logger.info("Sending SelectRoomType: {}", args[1]);
            auto room_res = m_resources->GetByName<core::RoomTypeMeta>(args[1]);

            input::ToolEvent tool_ev{};
            tool_ev.id = m_root_tool->Registry().GetOrAddId("RoomInput.SelectRoomType");
            tool_ev.payload = input::AnyPayload{
                .data = input::RoomInput::SelectRoomType{
                    .type = room_res.GetId()
                }
            };

            m_root_tool->Dispatch(tool_ev);
            
            // m_event_manager->FireSignal<RoomInputCommand>(
            //     [room_type = room_res.GetId()](RoomInput& ri) {
            //         ri.SelectRoomType(room_type);
            //     }
            // );
        } else if (args[0] == "EnterRoomMode") {
            m_logger.info("Making RoomInput active...");
            // m_event_manager->FireSignal<RoomInputCommand>([](RoomInput& ri) {
            //     ri.MakeActive();
            // });
            input::ToolEvent tool_ev{
                .id = m_root_tool->Registry().GetOrAddId("RootTool.EnterRoomTool")
            };
            m_root_tool->Dispatch(tool_ev);
        } else if (args[0] == "ExitRoomMode") {
            m_logger.info("Making RoomInput inactive...");
            m_root_tool->ReturnToSelf();
        }
    }
};

}