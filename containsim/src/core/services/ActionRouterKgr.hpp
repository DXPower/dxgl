#pragma once

#include <services/ActionRouter.hpp>
#include <kangaru_ext/kgr.hpp>

namespace services {
    struct ActionRouterService : kgr::autowire_service<ActionRouter> { };
    auto service_map(const ActionRouter&) -> ActionRouterService;
}