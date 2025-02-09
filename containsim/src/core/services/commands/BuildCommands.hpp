#pragma once

#include <services/commands/Command.hpp>
#include <common/Tile.hpp>

namespace services {
    class BuildManager;

    namespace commands {
        using BuildCommand = Command<BuildManager>;
        using BuildCommandPtr = CommandPtr<BuildCommand>;
        
        struct PlaceTile : BuildCommand {
            TileType type{};
            TileCoord coord{};

            void Execute(BuildManager& manager) const override;
        };
    }
}