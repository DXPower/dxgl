#pragma once

#include <services/commands/Command.hpp>
#include <common/Tile.hpp>

namespace services {
    class BuildManager;

    namespace commands {
        using BuildCommand = Command<BuildManager>;
        using BuildCommandPtr = CommandPtr<BuildCommand>;
        
        struct PlaceTiles : BuildCommand {
            TileType type{};
            TileCoord from{};
            TileCoord to{};

            void Execute(BuildManager& manager) const override;
        };

        struct DeleteTiles : BuildCommand {
            TileCoord from{};
            TileCoord to{};
            void Execute(BuildManager& manager) const override;
        };
    }
}