#pragma once

#include <kangaru/service.hpp>
#include <kangaru/autowire.hpp>

namespace kgr {
    template<typename... Overrides>
    struct autowire_override {
        template<typename T>
        struct Service : kgr::single_service<T, kgr::autowire>, kgr::overrides<kgr::mapped_service_t<Overrides>...> {};

        template<typename T>
        using mapped_service = Service<std::decay_t<T>>;
    };

    struct autowire_polymorphic {
        template<typename T>
        struct Service : kgr::single_service<T, kgr::autowire>, kgr::polymorphic {};

        template<typename T>
        using mapped_service = Service<std::decay_t<T>>;
    };

    template<typename T, typename Map = map<>>
    using service_for = mapped_service_t<T, Map>;
}