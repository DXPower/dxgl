#pragma once

#include <modules/core/Tile.hpp>
#include <modules/core/TileGrid.hpp>
#include <modules/application/EventManager.hpp>

namespace core {
    struct BuildCommand;

    class BuildManager {
        core::TileGrid* m_tile_grid{};

    public:
        BuildManager(core::TileGrid& tile_grid, application::EventManager& em);

        void PlaceTile(TileCoord coord, TileType type);
        void DeleteTopmostTile(TileCoord coord, TileLayer stop_at);

    private:
        void ProcessCommand(const BuildCommand& cmd);
    };

    struct BuildCommand {
        std::function<void(BuildManager&)> execute;
    };
}