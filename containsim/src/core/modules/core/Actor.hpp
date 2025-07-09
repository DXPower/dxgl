#pragma once

#include <cstdint>

namespace core {
    // While flecs has an id, it is not going to be consistent during a runtime.
    // This ID will be useful for persistent identification.
    using ActorId = std::uint64_t;
    struct Actor {
        ActorId id{};
    };
}