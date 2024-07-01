#include <systems/SpriteRenderer.hpp>
#include <kangaru_ext/kgr.hpp>

namespace systems {
    auto service_map(const SpriteRenderer&) -> kgr::autowire_single;
}