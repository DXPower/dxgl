#pragma once

#include <services/commands/Command.hpp>

namespace input {
class InputState;
}

namespace services {
    namespace commands {
        using InputStateCommand = Command<input::InputState>;
        using InputStateCommandPtr = CommandPtr<InputStateCommand>;

        struct InputStateEnterBuildMode : InputStateCommand {
            void Execute(input::InputState& input_state) const override;
        };

        struct InputStateEnterRoomMode : InputStateCommand {
            void Execute(input::InputState& input_state) const override;
        };

        struct InputStateExitMode : InputStateCommand {
            void Execute(input::InputState& input_state) const override;
        };
    }
}