#pragma once

#include <services/commands/Command.hpp>
#include <modules/core/Room.hpp>
#include <modules/core/Tile.hpp>

namespace core {
    class RoomManager;
}

namespace services {
    namespace commands {
        using RoomCommand = Command<core::RoomManager>;
        using RoomCommandPtr = CommandPtr<RoomCommand>;
        
        struct MarkRoom : RoomCommand {
            core::RoomType type{};
            core::TileSelection tiles{};

            void Execute(core::RoomManager& manager) const override;
        };

        struct UnmarkRoom : RoomCommand {
            core::TileSelection tiles{};

            void Execute(core::RoomManager& manager) const override;
        };
    }
}