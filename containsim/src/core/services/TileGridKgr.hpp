#pragma once

#include <services/TileGrid.hpp>
#include <kangaru_ext/kgr.hpp>

namespace services {
    auto service_map(const TileGrid&) -> kgr::autowire_single;
}