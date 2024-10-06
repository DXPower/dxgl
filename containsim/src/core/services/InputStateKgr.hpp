#include <services/InputState.hpp>
#include <kangaru_ext/kgr.hpp>

namespace services {
    auto service_map(const InputState&) -> kgr::autowire_single;
}