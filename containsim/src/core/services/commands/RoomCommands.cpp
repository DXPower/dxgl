#include <services/commands/RoomCommands.hpp>
#include <services/RoomManager.hpp>

using namespace services;
using namespace services::commands;

void MarkRoom::Execute(RoomManager& manager) const {
    manager.MarkTilesAsRoom(tiles, type);
}

void UnmarkRoom::Execute(RoomManager& manager) const {
    manager.UnmarkTiles(tiles);
}