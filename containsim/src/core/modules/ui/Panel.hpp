#pragma once

#include <services/Logging.hpp>
#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/ElementDocument.h>

// #include <services/ui/PanelCommands.hpp>
#include <modules/application/EventManager.hpp>

#include <string>

namespace ui {

class Panel {
    std::string m_name{};

protected:
    services::logging::Logger m_logger;

public:
    Panel(std::string name)
        : m_name(std::move(name)), m_logger(services::logging::CreateLogger("Panel" + m_name)) {
        // em.GetOrRegisterSignal<PanelCommand>()
        //     .signal.connect<&Panel::ProcessPanelEvent>(this);
    }

private:
    // void ProcessPanelEvent(const PanelCommand& cmd) {
    //     if (cmd.name != m_name)
    //         return;

    //     cmd.Execute(*this);
    // }
};

}