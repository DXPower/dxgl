#pragma once

#include <chrono>
namespace ch = std::chrono;

namespace core {
    // This is the real time since the last tick
    struct TimeSinceLastTick {
        ch::duration<float> value{}; // seconds
    };

    struct PauseTicks { };
}