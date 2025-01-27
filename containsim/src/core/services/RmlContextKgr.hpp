#pragma once

#include <RmlUi/Core/Context.h>
#include <kangaru_ext/kgr.hpp>

namespace services {    
    struct RmlContextService : kgr::extern_service<Rml::Context> { };
    auto service_map(const Rml::Context&) -> RmlContextService;
}