#pragma once

#include <glm/vec2.hpp>
#include <common/Rect.hpp>

namespace core {
    struct Transform {
        glm::vec2 position{};
        glm::vec2 size{};

        Rect ToRect() const {
            return Rect{
                .position = position, 
                .size = size};
        }
    };
}