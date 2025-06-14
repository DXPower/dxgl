#pragma once

#include <services/commands/Command.hpp>

namespace services {
    class InputState;

    namespace commands {
        using InputStateCommand = Command<InputState>;
        using InputStateCommandPtr = CommandPtr<InputStateCommand>;

        struct InputStateEnterBuildMode : InputStateCommand {
            void Execute(InputState& input_state) const override;
        };

        struct InputStateEnterRoomMode : InputStateCommand {
            void Execute(InputState& input_state) const override;
        };

        struct InputStateExitMode : InputStateCommand {
            void Execute(InputState& input_state) const override;
        };
    }
}