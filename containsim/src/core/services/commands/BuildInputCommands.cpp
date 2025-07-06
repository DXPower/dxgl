#include <services/commands/BuildInputCommands.hpp>
#include <modules/input/BuildInput.hpp>

using namespace services;
using namespace services::commands;
using namespace input;

void SelectTile::Execute(BuildInput& fsm) const {
    fsm.SelectTileToPlace(type);
}

void ExitBuildMode::Execute(BuildInput& fsm) const {
    fsm.ExitMode();
}

void ResetBuildInput::Execute(BuildInput& fsm) const {
    fsm.GetFsm().SetCurrentState(BuildInput::StateId::IdleMode);
}
