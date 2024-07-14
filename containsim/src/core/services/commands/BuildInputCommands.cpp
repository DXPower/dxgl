#include <services/commands/BuildInputCommands.hpp>
#include <services/BuildInput.hpp>

using namespace services;
using namespace services::commands;

void SelectTile::Execute(BuildInput& fsm) {
    fsm.SelectTileToPlace(type);
}

void EnterBuildMode::Execute(BuildInput& fsm) {
    fsm.EnterBuildMode();
}

void ExitBuildMode::Execute(BuildInput& fsm) {
    fsm.ExitMode();
}
