#pragma once

#include <flecs.h>
#include <glm/vec2.hpp>

namespace core {
    struct MapSize {
        glm::ivec2 value;
    };

    struct TileWorldSize {
        glm::vec2 value;
    };

    void RegisterCoreConfigurations(flecs::world& world);
}