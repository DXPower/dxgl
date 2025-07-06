#pragma once

#include <glm/vec2.hpp>


namespace application {
    struct WindowSizeChangedEvent {
        glm::ivec2 new_size{};
    };
}