#pragma once

#include <glm/vec2.hpp>

namespace services {
    enum class InputLayer {
        Offscreen, Game, Ui
    };

    struct IMouseTester {
        virtual ~IMouseTester() = default;

        virtual InputLayer TestMouse(glm::dvec2 pos) const = 0;
    };
}