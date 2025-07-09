#pragma once

#include <modules/core/Tile.hpp>
#include <modules/core/TileGrid.hpp>
#include <services/commands/CommandChains.hpp>
#include <services/commands/BuildCommands.hpp>

namespace core {
    class BuildManager : public services::commands::CommandConsumer<services::commands::BuildCommand> {
        core::TileGrid* m_tile_grid{};

    public:
        BuildManager(core::TileGrid& tile_grid);

        void PlaceTile(TileCoord coord, TileType type);
        void DeleteTopmostTile(TileCoord coord, TileLayer stop_at);

        void Consume(services::commands::BuildCommandPtr&& cmd) override;
    };
}