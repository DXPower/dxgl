#pragma once

#include <services/commands/Command.hpp>

namespace services {
    class InputState;

    namespace commands {
        using InputStateCommand = Command<InputState>;
        using InputStateCommandPtr = CommandPtr<InputStateCommand>;

        struct InputStateEnterBuildMode : InputStateCommand {
            void Execute(InputState& input_state) override;
        };

        struct InputStateExitMode : InputStateCommand {
            void Execute(InputState& input_state) override;
        };
    }
}