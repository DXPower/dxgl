#pragma once

#include <common/Pathing.hpp>
#include <components/Pathing.hpp>
#include <services/TileGrid.hpp>

#include <flecs.h>

namespace systems {
    class Pathfinder {
        [[maybe_unused]] const services::TileGrid* m_tile_grid{};

    public:
        Pathfinder(const services::TileGrid& tile_grid) : m_tile_grid(&tile_grid) { }

        // All positions should be in world pos
        PathPoints FindPath(const glm::vec2& start, const glm::vec2& end) const;

        void PreUpdate(flecs::world& world) const;
    };
}