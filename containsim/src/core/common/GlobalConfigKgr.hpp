#pragma once

#include <common/GlobalConfig.hpp>
#include <kangaru_ext/kgr.hpp>

struct GlobalConfigService : kgr::single_service<GlobalConfig> { };
auto service_map(const GlobalConfig&) -> GlobalConfigService;