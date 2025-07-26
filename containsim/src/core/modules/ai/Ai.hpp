#pragma once

#include <modules/ai/Perform.hpp>
#include <modules/ai/Utility.hpp>
#include <flecs.h>

namespace ai {
    class Ai {
    public:
        Ai(flecs::world& world);
    };
}