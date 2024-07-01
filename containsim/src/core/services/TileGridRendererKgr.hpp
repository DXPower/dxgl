#pragma once

#include <services/TileGridRenderer.hpp>
#include <kangaru_ext/kgr.hpp>

namespace services {
    auto service_map(const TileGridRenderer&) -> kgr::autowire_single;
}