#pragma once

#include <modules/ui/UiEvents.hpp>
#include <RmlUi/Core/EventListenerInstancer.h>
#include <RmlUi/Core/EventListener.h>
#include <common/Logging.hpp>
#include <modules/application/EventManager.hpp>

#include <memory>
#include <string>
#include <vector>

namespace ui {

class RmlEventManager : public Rml::EventListenerInstancer {
    class InlineEventForwarder : public Rml::EventListener {
        RmlEventManager* m_event_manager{};
        std::shared_ptr<std::vector<std::string>> m_args{};

    public:
        InlineEventForwarder(RmlEventManager* manager, const std::string& event_value) 
            : m_event_manager(manager) {
                m_args = std::make_shared<std::vector<std::string>>();
                Rml::StringUtilities::ExpandString(*m_args, event_value, ' ');
            }

        void ProcessEvent(Rml::Event& event) override {
            m_event_manager->ProcessEvent(event, m_args);
        }

        void OnDetach(Rml::Element*) override {
            delete this;
        }
    };

    logging::Logger m_logger{logging::CreateLogger("RmlEventManager")};
    application::EventSignal<ui_events::ElementEvent>* m_element_event_signal{}; 
public:
    RmlEventManager(application::EventManager& em) {
        m_element_event_signal = &em.GetOrRegisterSignal<ui_events::ElementEvent>();
    }

    Rml::EventListener* InstanceEventListener(const std::string& value, Rml::Element*) override {
        return new InlineEventForwarder(this, value);
    }

private:
    void ProcessEvent(Rml::Event& event, std::shared_ptr<const std::vector<std::string>> args) {
        if (args->size() == 0)
            return;

        // Broadcast event to all listeners
        m_element_event_signal->signal.fire(ui_events::ElementEvent{&event, std::move(args)});
    }
};

}