#pragma once

#include <services/commands/Command.hpp>
#include <common/Tile.hpp>

namespace input {
    class BuildInput;
}

namespace services {
    namespace commands {
        using BuildInputCommand = Command<input::BuildInput>;
        using BuildInputCommandPtr = CommandPtr<BuildInputCommand>;

        struct SelectTile : BuildInputCommand {
            TileType type{};
            
            void Execute(input::BuildInput& fsm) const override;
        };

        struct ExitBuildMode : BuildInputCommand {
            void Execute(input::BuildInput& fsm) const override;
        };

        // These are implemented the same for now
        using ExitDeleteMode = ExitBuildMode;

        struct ResetBuildInput : BuildInputCommand {
            void Execute(input::BuildInput& fsm) const override;
        };
    }
}