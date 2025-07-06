#pragma once

#include <services/commands/Command.hpp>
#include <common/Room.hpp>

namespace input {
class RoomInput;
}

namespace services {

    namespace commands {
        using RoomInputCommand = Command<input::RoomInput>;
        using RoomInputCommandPtr = CommandPtr<RoomInputCommand>;

        struct SelectRoomType : RoomInputCommand {
            RoomType type{};
            
            void Execute(input::RoomInput& fsm) const override;
        };

        struct SelectRoomClear : RoomInputCommand {
            void Execute(input::RoomInput& fsm) const override;
        };

        struct ExitRoomMode : RoomInputCommand {
            void Execute(input::RoomInput& fsm) const override;
        };

        struct ResetRoomInput : RoomInputCommand {
            void Execute(input::RoomInput& fsm) const override;
        };
    }
}