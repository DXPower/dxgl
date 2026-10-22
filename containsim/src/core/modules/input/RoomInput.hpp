#pragma once

#include <modules/core/Room.hpp>
#include <modules/application/EventManager.hpp>
#include <common/RoomInputEvents.hpp>
#include <common/Logging.hpp>
#include <modules/input/InputTool.hpp>

namespace input {

class RoomInput final : public InputTool {
    logging::Logger m_logger{logging::CreateLogger("RoomInput")};
    application::EventManager* m_em{};

    class DemarcationTool final : public InputTool {
        logging::Logger m_logger{logging::CreateLogger("RoomInput.DemarcationTool")};
        application::EventManager* m_em{};

        std::optional<core::RoomType> m_selected_type{};
    public:
        int m_action_select_type{};
        int m_action_clear{};

        DemarcationTool(application::EventManager& em, IdRegistry& registry);

    private:
        void MarkTiles(const core::TileSelection& selection);

    protected:
        void OnEnter() override;
        void OnExit() override;

        void OnClick(InputTool&, const ToolEvent& event, ToolEventControl&);
    } m_demarcation_tool;

public:
    RoomInput(application::EventManager& em, IdRegistry& registry);

    void OnEnter() override;
    void OnExit() override;

    struct SelectRoomType {
        core::RoomType type{};
    };
};

}
