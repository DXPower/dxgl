#pragma once

#include <RmlUi/Core/Event.h>
#include <string_view>

namespace services {
namespace ui {

enum class EventProc {
    Consumed,
    NotConsumed
};

class EventListener {
public:
    virtual ~EventListener() {}

    virtual EventProc ProcessUiEvent(Rml::Event& event, std::string_view value) = 0;
};

}
}