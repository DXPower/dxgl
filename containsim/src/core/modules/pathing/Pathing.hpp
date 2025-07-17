#pragma once

#include <modules/pathing/PathingComponents.hpp>
#include <flecs.h>

namespace pathing {
    class Pathing {
    public:
        Pathing(flecs::world& world);
    };
}