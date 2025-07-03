#pragma once

#include <common/ui/UiEvents.hpp>
#include <RmlUi/Core/EventListenerInstancer.h>
#include <RmlUi/Core/EventListener.h>
#include <services/Logging.hpp>
#include <modules/core/EventManager.hpp>

#include <string>
#include <span>
#include <vector>

namespace services {
namespace ui {

class RmlEventManager : public Rml::EventListenerInstancer {
    class InlineEventForwarder : public Rml::EventListener {
        RmlEventManager* m_event_manager{};
        std::vector<std::string> m_args{};

    public:
        InlineEventForwarder(RmlEventManager* manager, const std::string& event_value) 
            : m_event_manager(manager) {
                Rml::StringUtilities::ExpandString(m_args, event_value, ' ');
            }

        void ProcessEvent(Rml::Event& event) override {
            m_event_manager->ProcessEvent(event, m_args);
        }

        void OnDetach(Rml::Element*) override {
            delete this;
        }
    };

    logging::Logger m_logger{logging::CreateLogger("RmlEventManager")};
    core::EventSignal<ui_events::ElementEvent>* m_element_event_signal{}; 
public:
    RmlEventManager(core::EventManager& em) {
        m_element_event_signal = &em.GetOrRegisterSignal<ui_events::ElementEvent>();
    }

    Rml::EventListener* InstanceEventListener(const std::string& value, Rml::Element*) override {
        return new InlineEventForwarder(this, value);
    }

private:
    void ProcessEvent(Rml::Event& event, std::span<const std::string> args) {
        if (args.size() == 0)
            return;

        if (args[0] == "bc") {
            // Broadcast event to all listeners
            m_element_event_signal->signal.fire(ui_events::ElementEvent{&event, args.subspan(1)});
        }
    }
};

}
}