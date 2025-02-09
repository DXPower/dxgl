#pragma once

#include <services/Logging.hpp>
#include <services/ui/EventListener.hpp>
#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/ElementDocument.h>

#include <services/ui/PanelCommands.hpp>
#include <services/EventManager.hpp>

#include <stdexcept>
#include <string>

namespace services {
namespace ui {

class Panel {
    std::string m_name{};
    Rml::Element* m_panel_root{};

protected:
    logging::Logger m_logger;

public:
    // Name must match an element's ID within document
    Panel(std::string name, services::EventManager& em, Rml::ElementDocument& document)
        : m_name(std::move(name)), m_logger(logging::CreateLogger("Panel" + m_name)) {

        em.GetOrRegisterSignal<PanelCommand>()
            .signal.connect<&Panel::ProcessPanelEvent>(this);

        m_panel_root = document.GetElementById(m_name);

        if (m_panel_root == nullptr) {
            throw std::runtime_error("Failed to find panel root element: " + m_name);
        }

        if (!m_panel_root->HasAttribute("panel-active")) {
            m_panel_root->SetAttribute("panel-active", false);
        }
    }


    void Show() {
        m_panel_root->SetAttribute("panel-active", true);
    }

    void Hide() {
        m_panel_root->SetAttribute("panel-active", false);
    }

private:
    void ProcessPanelEvent(const PanelCommand& cmd) {
        if (cmd.name != m_name)
            return;

        cmd.Execute(*this);
    }
};

}
}