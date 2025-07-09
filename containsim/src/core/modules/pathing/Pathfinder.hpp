#pragma once

#include <common/Paths.hpp>
#include <modules/core/TileGrid.hpp>

#include <flecs.h>

namespace pathing {
    class Pathfinder {
        [[maybe_unused]] const core::TileGrid* m_tile_grid{};

    public:
        Pathfinder(const core::TileGrid& tile_grid) : m_tile_grid(&tile_grid) { }

        // All positions should be in world pos
        PathPoints FindPath(const glm::vec2& start, const glm::vec2& end) const;

        void PreUpdate(flecs::world& world) const;

    private:
        std::vector<core::TileCoord> AStar(const core::TileCoord& start, const core::TileCoord& end) const;
    };
}