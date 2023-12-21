#pragma once

#include <services/commands/ServiceCommands.hpp>

#include <common/Tile.hpp>

namespace services::commands {
    struct SelectTile : BuildCommand {
        TileType type{};

        void Execute(BuildManager& manager) override;
    };

    struct ExitBuildMode : BuildCommand {
        void Execute(BuildManager& manager) override;
    };
}