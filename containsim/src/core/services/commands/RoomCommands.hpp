#pragma once

#include <services/commands/Command.hpp>
#include <common/Room.hpp>
#include <common/Tile.hpp>

namespace core {
    class RoomManager;
}

namespace services {
    namespace commands {
        using RoomCommand = Command<core::RoomManager>;
        using RoomCommandPtr = CommandPtr<RoomCommand>;
        
        struct MarkRoom : RoomCommand {
            RoomType type{};
            TileSelection tiles{};

            void Execute(core::RoomManager& manager) const override;
        };

        struct UnmarkRoom : RoomCommand {
            TileSelection tiles{};

            void Execute(core::RoomManager& manager) const override;
        };
    }
}