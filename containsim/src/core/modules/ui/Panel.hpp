#pragma once

#include <common/Logging.hpp>
#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/ElementDocument.h>

// #include <services/ui/PanelCommands.hpp>
#include <modules/application/EventManager.hpp>

#include <string>

namespace ui {

class Panel {
    std::string m_name{};

protected:
    logging::Logger m_logger;

public:
    Panel(std::string name)
        : m_name(std::move(name)), m_logger(logging::CreateLogger("Panel" + m_name)) {
    }
};

}