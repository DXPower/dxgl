#pragma once

#include <flecs.h>
#include <mutex>

namespace components {
    struct ComponentProducer {
        flecs::entity event_component{};
    };

    namespace detail {
        template<typename T>
        struct ProducedComponent {
            flecs::entity entity{};
            std::once_flag init_flag{};
        };

        template<typename T>
        ProducedComponent<T> produced_component{};
    }

    template<typename T>
    ComponentProducer MakeComponentProducer(flecs::world& world) {
        std::call_once(detail::produced_component<T>.init_flag, [&]() {
            detail::produced_component<T>.entity = world.component<T>();
        });

        return ComponentProducer{detail::produced_component<T>.entity};
    }
}