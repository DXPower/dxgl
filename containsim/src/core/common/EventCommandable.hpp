#pragma once

#include <common/Command.hpp>
#include <modules/application/EventManager.hpp>

template<typename T>
class EventCommandable : public Commandable<T> {
protected:
    EventCommandable(application::EventManager& em) {
        RegisterEvent(em);
    }

private:
    void RegisterEvent(application::EventManager& em) {
        em.GetOrRegisterSignal<Command<T>>().signal.template connect<&EventCommandable<T>::ProcessCommand>(static_cast<T&>(*this));
    }
};
