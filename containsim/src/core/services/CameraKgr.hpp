#pragma once

#include <services/Camera.hpp>
#include <kangaru_ext/kgr.hpp>

namespace services {
    auto service_map(const Camera&) -> kgr::autowire_single;
}