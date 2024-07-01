#pragma once

#include <services/commands/Command.hpp>

namespace services {
    class GameState;

    namespace commands {
        using StateCommand = Command<GameState>;
        using StateCommandPtr = CommandPtr<StateCommand>;

        struct StateEnterBuildMode : StateCommand {
            void Execute(GameState& game_state) override;
        };

        struct StateExitMode : StateCommand {
            void Execute(GameState& game_state) override;
        };
    }
}