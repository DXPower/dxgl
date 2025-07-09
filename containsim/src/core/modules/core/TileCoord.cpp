#include <modules/core/TileCoord.hpp>

using namespace core;

TileCoordNeighbors core::GetBoundlessTileCoordNeighbors(TileCoord coord) {
    TileCoordNeighbors neighbors{};

    neighbors.west = {coord.x - 1, coord.y};
    neighbors.northwest = {coord.x - 1, coord.y - 1};
    neighbors.southwest = {coord.x - 1, coord.y + 1};
    neighbors.east = {coord.x + 1, coord.y};
    neighbors.northeast = {coord.x + 1, coord.y - 1};
    neighbors.southeast = {coord.x + 1, coord.y + 1};
    neighbors.north = {coord.x, coord.y - 1};
    neighbors.south = {coord.x, coord.y + 1};

    return neighbors;
}