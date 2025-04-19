#pragma once

#include <components/CircleMover.hpp>
#include <components/Transform.hpp>

#include <flecs.h>

#include <cmath>

namespace systems {
    inline void CircleMover(flecs::iter& it, 
        size_t,
        components::CircleMover& mover,
        components::Transform& transform) {
        
        glm::vec2 new_pos = mover.center + mover.radius * glm::vec2(
            std::cos(mover.time),
            std::sin(mover.time)
        );

        transform.position = new_pos;
        mover.time += mover.angular_velocity * it.delta_time();
    }
}