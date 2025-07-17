#pragma once

#include <glm/vec2.hpp>
#include <boost/container/small_vector.hpp>
#include <optional>

namespace core {
using TileCoord = glm::ivec2;

struct TileCoordNeighbors {
    using NeighborList = boost::container::small_vector<TileCoord, 8>;

    std::optional<TileCoord> north, northeast, east, southeast,
        south, southwest, west, northwest;

    NeighborList ToList() const;
};

TileCoordNeighbors GetBoundlessTileCoordNeighbors(TileCoord coord);
}