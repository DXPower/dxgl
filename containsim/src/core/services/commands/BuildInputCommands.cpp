#include <services/commands/BuildInputCommands.hpp>
#include <services/BuildInput.hpp>

using namespace services;
using namespace services::commands;

void SelectTile::Execute(BuildInput& fsm) {
    fsm.SelectTileToPlace(type);
}

void ExitBuildMode::Execute(BuildInput& fsm) {
    fsm.ExitMode();
}
