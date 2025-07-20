#pragma once

#include <common/Ticks.hpp>

namespace core {
    struct Cooldown {
        TickDuration time_remaining{};
    };
}