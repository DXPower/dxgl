#pragma once

#include <modules/ai/Perform.hpp>
#include <flecs.h>

namespace ai {
    class Ai {
    public:
        Ai(flecs::world& world);
    };
}