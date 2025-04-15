#include <services/commands/BuildCommands.hpp>
#include <services/BuildManager.hpp>

#include <algorithm>

namespace services{
namespace commands {

void PlaceTiles::Execute(BuildManager& manager) const {
    auto [min_x, max_x] = std::minmax(from.x, to.x);
    auto [min_y, max_y] = std::minmax(from.y, to.y);

    for (auto x = min_x; x <= max_x; x++) {
        for (auto y = min_y; y <= max_y; y++) {
            manager.PlaceTile(TileCoord{x, y}, type);
        }
    }
}

void DeleteTiles::Execute(BuildManager& manager) const {
    auto [min_x, max_x] = std::minmax(from.x, to.x);
    auto [min_y, max_y] = std::minmax(from.y, to.y);

    for (auto x = min_x; x <= max_x; x++) {
        for (auto y = min_y; y <= max_y; y++) {
            manager.DeleteTopmostTile(TileCoord{x, y}, TileLayer::Walls);
        }
    }
}

}
}