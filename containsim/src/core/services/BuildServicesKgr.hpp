#include <services/BuildInput.hpp>
#include <services/BuildManager.hpp>

#include <kangaru_ext/kgr.hpp>

namespace services {
    auto service_map(const BuildInput&) -> kgr::autowire_single;
    auto service_map(const BuildManager&) -> kgr::autowire_single;
}