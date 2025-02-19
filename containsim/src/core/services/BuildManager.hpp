#pragma once

#include <common/Tile.hpp>
#include <services/GameState.hpp>
#include <services/TileGrid.hpp>
#include <services/commands/CommandChains.hpp>
#include <services/commands/BuildCommands.hpp>

namespace services {
    class BuildManager : public commands::CommandConsumer<commands::BuildCommand> {
        TileGrid* m_tile_grid{};

    public:
        BuildManager(TileGrid& tile_grid);

        void PlaceTile(TileCoord coord, TileType type);
        void DeleteTile(TileCoord coord);

        void Consume(commands::BuildCommandPtr&& cmd) override;
    };
}