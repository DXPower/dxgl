#pragma once

#include <services/commands/Command.hpp>
#include <common/Tile.hpp>

#include <memory>

namespace services {
    class BuildMode;

    namespace commands {
        using BuildModeCommand = Command<BuildMode>;
        using BuildModeCommandPtr = CommandPtr<BuildModeCommand>;

        struct SelectTile : BuildModeCommand {
            TileType type{};

            void Execute(BuildMode& mode) override;
        };

        struct EnterBuildMode : BuildModeCommand {
            void Execute(BuildMode& mode) override;
        };

        struct ExitBuildMode : BuildModeCommand {
            void Execute(BuildMode& mode) override;
        };
    }
}