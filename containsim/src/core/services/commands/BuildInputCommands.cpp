#include <services/commands/BuildInputCommands.hpp>
#include <services/BuildInput.hpp>

using namespace services;
using namespace services::commands;

void SelectTile::Execute(BuildInput& fsm) const {
    fsm.SelectTileToPlace(type);
}

void ExitBuildMode::Execute(BuildInput& fsm) const {
    fsm.ExitMode();
}

void ResetBuildInput::Execute(BuildInput& fsm) const {
    fsm.GetFsm().SetCurrentState(BuildInput::StateId::IdleMode);
}
