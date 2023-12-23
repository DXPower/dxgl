#pragma once

#include <glm/vec2.hpp>

enum class TileType {
    Grass,
    Dirt,
    Tile
};

struct TileData {
    TileType type{};

    constexpr bool operator==(const TileData& rhs) const = default;
};

using TileCoord = glm::ivec2;

struct Tile {
    TileCoord coord{};
    TileData data{};
};