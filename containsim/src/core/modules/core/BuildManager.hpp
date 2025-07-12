#pragma once

#include <modules/core/Tile.hpp>
#include <modules/core/TileGrid.hpp>
#include <modules/application/EventManager.hpp>
#include <common/EventCommandable.hpp>

namespace core {
    class BuildManager : public EventCommandable<BuildManager> {
        core::TileGrid* m_tile_grid{};

    public:
        BuildManager(core::TileGrid& tile_grid, application::EventManager& em);

        void PlaceTile(TileCoord coord, TileType type);
        void DeleteTopmostTile(TileCoord coord, TileLayer stop_at);
    };

    using BuildCommand = Command<BuildManager>;
}