#include <services/commands/RoomCommands.hpp>
#include <modules/core/RoomManager.hpp>

using namespace services;
using namespace services::commands;

void MarkRoom::Execute(core::RoomManager& manager) const {
    manager.MarkTilesAsRoom(tiles, type);
}

void UnmarkRoom::Execute(core::RoomManager& manager) const {
    manager.UnmarkTiles(tiles);
}