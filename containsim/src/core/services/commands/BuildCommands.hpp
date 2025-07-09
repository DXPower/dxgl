#pragma once

#include <services/commands/Command.hpp>
#include <modules/core/Tile.hpp>

namespace core {
    class BuildManager;
}

namespace services {

    namespace commands {
        using BuildCommand = Command<core::BuildManager>;
        using BuildCommandPtr = CommandPtr<BuildCommand>;
        
        struct PlaceTiles : BuildCommand {
            core::TileType type{};
            core::TileCoord from{};
            core::TileCoord to{};

            void Execute(core::BuildManager& manager) const override;
        };

        struct DeleteTiles : BuildCommand {
            core::TileCoord from{};
            core::TileCoord to{};
            void Execute(core::BuildManager& manager) const override;
        };
    }
}