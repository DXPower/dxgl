#pragma once

#include <services/commands/Command.hpp>
#include <common/Room.hpp>
#include <common/Tile.hpp>

namespace services {
    class RoomManager;

    namespace commands {
        using RoomCommand = Command<RoomManager>;
        using RoomCommandPtr = CommandPtr<RoomCommand>;
        
        struct MarkRoom : RoomCommand {
            RoomType type{};
            TileSelection tiles{};

            void Execute(RoomManager& manager) const override;
        };

        struct UnmarkRoom : RoomCommand {
            TileSelection tiles{};

            void Execute(RoomManager& manager) const override;
        };
    }
}