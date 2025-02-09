#include <services/ui/PanelCommands.hpp>
#include <services/ui/Panel.hpp>

using namespace services::ui;

void ShowPanel::Execute(Panel& panel) const {
    panel.Show();
}

void HidePanel::Execute(Panel& panel) const {
    panel.Hide();
}