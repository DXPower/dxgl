#pragma once

#include <kangaru_ext/kgr.hpp>
#include <services/InputHandler.hpp>

namespace services {
    auto service_map(const InputHandler&) -> kgr::autowire_single;
}