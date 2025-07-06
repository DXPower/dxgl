#pragma once

#include <services/commands/Command.hpp>
#include <string>

namespace ui {

class Panel;

struct PanelCommand : services::commands::Command<Panel> {
    std::string name{};
};

struct ShowPanel : PanelCommand {
    void Execute(Panel& panel) const override;
};

struct HidePanel : PanelCommand {
    void Execute(Panel& panel) const override;
};

}