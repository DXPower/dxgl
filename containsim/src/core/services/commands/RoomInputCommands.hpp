#pragma once

#include <services/commands/Command.hpp>
#include <common/Room.hpp>

namespace services {
    class RoomInput;

    namespace commands {
        using RoomInputCommand = Command<RoomInput>;
        using RoomInputCommandPtr = CommandPtr<RoomInputCommand>;

        struct SelectRoomType : RoomInputCommand {
            RoomType type{};
            
            void Execute(RoomInput& fsm) const override;
        };

        struct SelectRoomClear : RoomInputCommand {
            void Execute(RoomInput& fsm) const override;
        };

        struct ExitRoomMode : RoomInputCommand {
            void Execute(RoomInput& fsm) const override;
        };

        struct ResetRoomInput : RoomInputCommand {
            void Execute(RoomInput& fsm) const override;
        };
    }
}