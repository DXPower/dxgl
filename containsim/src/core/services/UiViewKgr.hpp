#pragma once

#include <services/UiView.hpp>
#include <services/interfaces/IMouseTesterKgr.hpp>
#include <kangaru_ext/kgr.hpp>

namespace services {
    struct UiViewService : kgr::extern_service<UiView>, kgr::overrides<MouseTesterService> { };
    auto service_map(const UiView&) -> UiViewService;
}