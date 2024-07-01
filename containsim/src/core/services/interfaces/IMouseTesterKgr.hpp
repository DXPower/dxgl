#pragma once

#include <services/interfaces/IMouseTester.hpp>
#include <kangaru_ext/kgr.hpp>

namespace services {
    struct MouseTesterService : kgr::abstract_service<IMouseTester> { };
    auto service_map(const IMouseTester&) -> MouseTesterService;
}