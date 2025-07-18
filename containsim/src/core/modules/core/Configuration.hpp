#pragma once

#include <flecs.h>
#include <glm/vec2.hpp>

#include <chrono>
namespace ch = std::chrono;

namespace core {
    struct MapSize {
        glm::ivec2 value;
    };

    struct TileWorldSize {
        glm::vec2 value;
    };

    // How often a tick occurs
    struct TickRate {
        ch::duration<float> period{}; // seconds
    };

    void RegisterCoreConfigurations(flecs::world& world);
}