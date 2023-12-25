#pragma once

#include <glm/vec2.hpp>

namespace components {
    struct Transform {
        glm::vec2 position{};
        glm::vec2 size{};
        glm::vec2 rotation{};
    };
}