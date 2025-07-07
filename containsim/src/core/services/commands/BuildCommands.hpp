#pragma once

#include <services/commands/Command.hpp>
#include <common/Tile.hpp>

namespace core {
    class BuildManager;
}

namespace services {

    namespace commands {
        using BuildCommand = Command<core::BuildManager>;
        using BuildCommandPtr = CommandPtr<BuildCommand>;
        
        struct PlaceTiles : BuildCommand {
            TileType type{};
            TileCoord from{};
            TileCoord to{};

            void Execute(core::BuildManager& manager) const override;
        };

        struct DeleteTiles : BuildCommand {
            TileCoord from{};
            TileCoord to{};
            void Execute(core::BuildManager& manager) const override;
        };
    }
}