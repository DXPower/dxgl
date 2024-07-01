#pragma once

#include <dxgl/Application.hpp>
#include <kangaru/kangaru.hpp>
#include <kangaru_ext/kgr.hpp>

namespace services {
    struct WindowService 
        : kgr::single_service<dxgl::Window>
        , kgr::polymorphic
        , kgr::supplied
    { };

    struct SubWindowService 
        : kgr::single_service<dxgl::SubWindow>
        , kgr::overrides<WindowService>
        , kgr::supplied
    { };
};

namespace dxgl {
    auto service_map(const Window&) -> services::WindowService;
}