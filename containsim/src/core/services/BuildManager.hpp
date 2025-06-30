#pragma once

#include <common/Tile.hpp>
#include <modules/core/TileGrid.hpp>
#include <services/commands/CommandChains.hpp>
#include <services/commands/BuildCommands.hpp>

namespace services {
    class BuildManager : public commands::CommandConsumer<commands::BuildCommand> {
        core::TileGrid* m_tile_grid{};

    public:
        BuildManager(core::TileGrid& tile_grid);

        void PlaceTile(TileCoord coord, TileType type);
        void DeleteTopmostTile(TileCoord coord, TileLayer stop_at);

        void Consume(commands::BuildCommandPtr&& cmd) override;
    };
}