#pragma once

#include <glm/vec2.hpp>

namespace components {
    struct CircleMover {
        glm::vec2 center{};
        float angular_velocity{};
        float radius{};
        float time{};
    };
}