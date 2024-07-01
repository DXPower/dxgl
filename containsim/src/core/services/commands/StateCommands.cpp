#include <services/commands/StateCommands.hpp>
#include <services/GameState.hpp>

using namespace services;
using namespace commands;

void StateEnterBuildMode::Execute(GameState& game_state) {
    game_state.EnterBuildMode();
}

void StateExitMode::Execute(GameState& game_state) {
    game_state.ExitMode();
}