#pragma once

#include <nano/nano_signal_slot.hpp>
#include <common/Logging.hpp>

#include <concepts>
#include <string_view>
#include <typeinfo>
#include <type_traits>

namespace application {
namespace detail {
    struct EventSignalBase {
        virtual ~EventSignalBase() = default;
    };

    template<typename T>
    concept EventReturnType = requires {
        typename T::ReturnType;
    };
}

template<typename T>
struct EventSignal : detail::EventSignalBase {
    Nano::Signal<void(const T&)> signal{};
};

template<typename T>
requires detail::EventReturnType<T>
struct EventSignal<T> : detail::EventSignalBase {
    std::string name{};
    Nano::Signal<typename T::ReturnType(const T&)> signal{};
};

class EventManager {
    std::unordered_map<std::size_t, std::unique_ptr<detail::EventSignalBase>> m_signals{};
    logging::Logger m_logger{logging::CreateLogger("EventManager")};

public:
    EventManager() = default;
    EventManager(const EventManager&) = delete;
    EventManager(EventManager&&) noexcept = default;
    EventManager& operator=(const EventManager&) = delete;
    EventManager& operator=(EventManager&&) = default;

    template<typename T>
    EventSignal<T>& RegisterSignal() {
        auto signal = std::make_unique<EventSignal<T>>();
        auto [it, ins] = m_signals.emplace(typeid(EventSignal<T>).hash_code(), std::move(signal));
        return static_cast<EventSignal<T>&>(*it->second);
    }

    template<typename T>
    auto& GetSignal(this auto&& self) {
        auto it = self.m_signals.find(typeid(EventSignal<T>).hash_code());

        if (it == self.m_signals.end()) {
            throw std::runtime_error("Event signal not found");
        }

        using Signal_t = std::conditional_t<
            std::is_const_v<std::remove_reference_t<decltype(self)>>,
            const EventSignal<T>,
            EventSignal<T>
        >;

        auto signal = dynamic_cast<Signal_t*>(it->second.get());
        if (signal == nullptr) {
            throw std::runtime_error(std::format("Event signal type mismatch: {}", typeid(T).name()));
        }

        return *signal;
    }

    template<typename T>
    EventSignal<T>& GetOrRegisterSignal() {
        auto it = m_signals.find(typeid(EventSignal<T>).hash_code());

        if (it == m_signals.end()) {
            return RegisterSignal<T>();
        } else {
            return *dynamic_cast<EventSignal<T>*>(it->second.get());
        }
    }

    template<typename T>
    void FireSignal(const T& e) {
        GetSignal<T>().signal.fire(e);
    }

    template<typename T>
    void RemoveSignal() {
        m_signals.erase(typeid(EventSignal<T>).hash_code());
    }
};

}