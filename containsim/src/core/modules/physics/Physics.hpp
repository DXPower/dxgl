#pragma once

#include <modules/physics/Collider.hpp>
#include <flecs.h>

namespace physics {
    class Physics {
    public:
        Physics(flecs::world& world);
    };
}