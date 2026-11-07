#pragma once

#include <modules/core/Tile.hpp>
#include <modules/application/EventManager.hpp>
#include <common/BuildInputEvents.hpp>
#include <common/Logging.hpp>
#include <modules/input/InputTool.hpp>

namespace input {

class BuildInput final : public InputTool {
    logging::Logger m_logger{logging::CreateLogger("BuildInput")};
    application::EventManager* m_em{};
    
    class EditingTool final : public InputTool {
        logging::Logger m_logger{logging::CreateLogger("BuildInput.EditingTool")};
        application::EventManager* m_em{};
        
        std::optional<core::TileType> m_selected_type{};
    public:
        int m_action_select_type{};
        int m_action_clear{};

        EditingTool(application::EventManager& em, IdRegistry& registry);

    private:
        void EditTiles(const core::TileSelection& selection);

    protected:
        void OnEnter() override;
        void OnExit() override;

        void OnClick(InputTool&, const ToolEvent& event, ToolEventControl&);
    } m_editing_tool;

public:
    BuildInput(application::EventManager& em, IdRegistry& registry);

    void OnEnter() override;
    void OnExit() override;

    struct SelectTileType {
        core::TileType type{};
    };
};

}
