#pragma once

#include <dxgl/Ubo.hpp>
#include <kangaru_ext/kgr.hpp>

struct UboService : kgr::single_service<dxgl::UboBindingManager> { };

namespace dxgl {
    auto service_map(const UboBindingManager&) -> UboService;
}