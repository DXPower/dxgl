#pragma once

#include <services/commands/Command.hpp>
#include <common/Tile.hpp>

namespace services {
    class BuildInput;

    namespace commands {
        using BuildInputCommand = Command<BuildInput>;
        using BuildInputCommandPtr = CommandPtr<BuildInputCommand>;

        struct SelectTile : BuildInputCommand {
            TileType type{};

            void Execute(BuildInput& fsm) override;
        };

        struct EnterBuildMode : BuildInputCommand {
            void Execute(BuildInput& fsm) override;
        };

        struct ExitBuildMode : BuildInputCommand {
            void Execute(BuildInput& fsm) override;
        };

        // These are implemented the same for now
        using ExitDeleteMode = ExitBuildMode;
    }
}