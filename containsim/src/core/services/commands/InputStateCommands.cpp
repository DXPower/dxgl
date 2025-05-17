#include <services/commands/InputStateCommands.hpp>
#include <services/InputState.hpp>

using namespace services;
using namespace commands;

void InputStateEnterBuildMode::Execute(InputState& input_state) const {
    input_state.EnterBuildMode();
}

void InputStateEnterRoomMode::Execute(InputState& input_state) const {
    input_state.EnterRoomMode();
}

void InputStateExitMode::Execute(InputState& input_state) const {
    input_state.ExitMode();
}