#pragma once

#include <glm/vec2.hpp>

namespace input {
    enum class InputLayer {
        Offscreen, Game
    };

    struct IMouseTester {
        virtual ~IMouseTester() = default;

        virtual InputLayer TestMouse(glm::dvec2 pos) const = 0;
    };
}